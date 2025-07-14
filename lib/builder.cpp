#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <ranges>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

llove::Builder::Builder(Context &types)
    : m_Types(types),
      m_Builder(m_Context),
      m_Module("main", m_Context)
{
}

llove::Context &llove::Builder::GetTypes() const
{
    return m_Types;
}

std::string llove::Builder::Mangle(
    const bool interface,
    const std::string &class_name,
    const bool mutable_,
    const std::string &name,
    const std::vector<Parameter> &parameters,
    const bool vararg,
    const Field &result) const
{
    if (interface)
        return name;

    auto mangled = '?' + std::to_string(name.size()) + '_' + name;

    if (!class_name.empty())
        mangled += (mutable_ ? 'm' : 'c') + std::to_string(class_name.size()) + '_' + class_name;

    if (vararg)
        mangled += 'v';

    mangled += std::to_string(parameters.size()) + '_';
    for (auto &[info_, name_] : parameters)
        mangled += info_.Mangle();

    return mangled + result.Mangle();
}

llvm::Type *llove::Builder::GetVoidType()
{
    return m_Builder.getVoidTy();
}

llvm::IntegerType *llove::Builder::GetIntType(const unsigned bits)
{
    return m_Builder.getIntNTy(bits);
}

llvm::Type *llove::Builder::GetFltType(const unsigned bits)
{
    switch (bits)
    {
    case 16:
        return m_Builder.getHalfTy();
    case 32:
        return m_Builder.getFloatTy();
    case 64:
        return m_Builder.getDoubleTy();
    default:
        return nullptr;
    }
}

llvm::ArrayType *llove::Builder::GetArrayType(llvm::Type *base, const unsigned size)
{
    return llvm::ArrayType::get(base, size);
}

llvm::PointerType *llove::Builder::GetPointerType()
{
    return llvm::PointerType::getUnqual(m_Context);
}

llvm::PointerType *llove::Builder::GetPointerType(llvm::Type *base)
{
    return llvm::PointerType::getUnqual(base);
}

llvm::StructType *llove::Builder::GetStructType(const std::vector<llvm::Type *> &fields, const bool packed)
{
    return llvm::StructType::get(m_Context, fields, packed);
}

llvm::FunctionType *llove::Builder::GetFunctionType(
    llvm::Type *result,
    const std::vector<llvm::Type *> &parameters,
    const bool vararg)
{
    return llvm::FunctionType::get(result, parameters, vararg);
}

llvm::StructType *llove::Builder::GetNamedStructType(const std::string &name)
{
    return llvm::StructType::getTypeByName(m_Context, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(const std::string &name)
{
    if (const auto type = llvm::StructType::getTypeByName(m_Context, name))
        return type;
    return llvm::StructType::create(m_Context, name);
}

llvm::StructType *llove::Builder::GetOrCreateNamedStructType(
    const std::string &name,
    const std::vector<llvm::Type *> &fields,
    const bool packed)
{
    if (const auto type = llvm::StructType::getTypeByName(m_Context, name))
    {
        type->setBody(fields, packed);
        return type;
    }
    return llvm::StructType::create(m_Context, fields, name, packed);
}

llvm::Value *llove::Builder::CreateAlloca(llvm::Function *parent, const TypePtr &type)
{
    const auto insert_block = GetInsertBlock();
    SetInsertPointPastAllocas(parent);
    const auto pointer = m_Builder.CreateAlloca(type->Gen(*this));
    SetInsertPoint(insert_block);
    return pointer;
}

llvm::Value *llove::Builder::CreateLoad(llvm::Value *pointer, const TypePtr &type)
{
    return m_Builder.CreateLoad(type->Gen(*this), pointer);
}

llvm::Value *llove::Builder::CreateStore(llvm::Value *pointer, llvm::Value *value, const bool volatile_)
{
    return m_Builder.CreateStore(value, pointer, volatile_);
}

void llove::Builder::CreateRetVoid()
{
    m_Builder.CreateRetVoid();
}

void llove::Builder::CreateRet(llvm::Value *value)
{
    m_Builder.CreateRet(value);
}

llvm::Value *llove::Builder::CreateCall(
    const FunctionType::Ptr &type,
    llvm::Value *callee,
    const std::vector<llvm::Value *> &arguments)
{
    return m_Builder.CreateCall(type->Gen(*this), callee, arguments);
}

llvm::Value *llove::Builder::CreateCall(const llvm::FunctionCallee callee, const std::vector<llvm::Value *> &arguments)
{
    return m_Builder.CreateCall(callee, arguments);
}

llvm::Value *llove::Builder::CreateInsertValue(llvm::Value *aggregate, llvm::Value *value, const unsigned index)
{
    return m_Builder.CreateInsertValue(aggregate, value, index);
}

llove::ValuePtr llove::Builder::CreatePointerOffset(const ValuePtr &pointer, const ValuePtr &offset)
{
    auto type = As<PointerType>(pointer->GetType());
    const auto element_pointer = m_Builder.CreateGEP(
        type->GetBase()->Gen(*this),
        pointer->Load(*this),
        offset->Load(*this));
    return Value::CreateR(std::move(type), element_pointer);
}

llove::ValuePtr llove::Builder::CreatePointerDifference(const ValuePtr &begin, const ValuePtr &end)
{
    const auto type = As<PointerType>(begin->GetType());
    const auto value = m_Builder.CreatePtrDiff(
        type->GetBase()->Gen(*this),
        begin->Load(*this),
        end->Load(*this));
    return Value::CreateR(m_Types.GetInteger(true, 64), value);
}

llove::ValuePtr llove::Builder::CreatePointerElement(const ValuePtr &pointer, const ValuePtr &index)
{
    const auto type = As<PointerType>(pointer->GetType());
    const auto value = m_Builder.CreateGEP(
        type->GetBase()->Gen(*this),
        pointer->Load(*this),
        index->Load(*this));
    return Value::CreateL(type->GetBase(), value, type->IsMutable());
}

llove::ValuePtr llove::Builder::CreateArrayElement(ValuePtr array, const ValuePtr &index)
{
    const auto type = As<ArrayType>(array->GetType());

    if (!array->IsReferenceable())
    {
        const auto storage = CreateAlloca(GetParent(), type);
        CreateStore(storage, array->Load(*this));
        array = Value::CreateL(type, storage, false);
    }

    const auto value = m_Builder.CreateGEP(
        type->GetBase()->Gen(*this),
        array->GetPointer(),
        index->Load(*this));
    return Value::CreateL(type->GetBase(), value, array->IsMutable());
}

llvm::Value *llove::Builder::CreateStructGEP(const TypePtr &type, llvm::Value *pointer, const unsigned index)
{
    return m_Builder.CreateStructGEP(type->Gen(*this), pointer, index);
}

llove::ValuePtr llove::Builder::CreateAdd(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateAdd(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateSub(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateSub(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateMul(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateMul(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateDiv(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_Builder.CreateSDiv(left->Load(*this), right->Load(*this));
    else
        value = m_Builder.CreateUDiv(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateRem(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_Builder.CreateSRem(left->Load(*this), right->Load(*this));
    else
        value = m_Builder.CreateURem(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateAnd(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateAnd(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateOr(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateOr(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateXor(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateXor(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateICmpEQ(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateICmpNE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpLT(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_Builder.CreateICmpSLT(left->Load(*this), right->Load(*this));
    else
        value = m_Builder.CreateICmpULT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpGT(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_Builder.CreateICmpSGT(left->Load(*this), right->Load(*this));
    else
        value = m_Builder.CreateICmpUGT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpLE(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_Builder.CreateICmpSLE(left->Load(*this), right->Load(*this));
    else
        value = m_Builder.CreateICmpULE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpGE(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_Builder.CreateICmpSGE(left->Load(*this), right->Load(*this));
    else
        value = m_Builder.CreateICmpUGE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFAdd(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFAdd(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFSub(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFSub(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFMul(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFMul(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFDiv(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFDiv(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFRem(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFRem(left->Load(*this), right->Load(*this));
    return Value::CreateR(left->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFCmpOEQ(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFCmpONE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpLT(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFCmpOLT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpGT(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFCmpOGT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpLE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFCmpOLE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpGE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_Builder.CreateFCmpOGE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreatePCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto left_int = CreateCast(left, m_Types.GetInteger(false, 64));
    const auto right_int = CreateCast(right, m_Types.GetInteger(false, 64));
    return CreateCmpEQ(left_int, right_int);
}

llove::ValuePtr llove::Builder::CreatePCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto left_int = CreateCast(left, m_Types.GetInteger(false, 64));
    const auto right_int = CreateCast(right, m_Types.GetInteger(false, 64));
    return CreateCmpNE(left_int, right_int);
}

llove::ValuePtr llove::Builder::CreateNeg(const ValuePtr &operand)
{
    const auto value = m_Builder.CreateNeg(operand->Load(*this));
    return Value::CreateR(operand->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFNeg(const ValuePtr &operand)
{
    const auto value = m_Builder.CreateFNeg(operand->Load(*this));
    return Value::CreateR(operand->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateNot(const ValuePtr &operand)
{
    const auto value = m_Builder.CreateIsNull(operand->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateInv(const ValuePtr &operand)
{
    const auto value = m_Builder.CreateNot(operand->Load(*this));
    return Value::CreateR(operand->GetType(), value);
}

void llove::Builder::CreateBranch(llvm::BasicBlock *block)
{
    m_Builder.CreateBr(block);
}

void llove::Builder::CreateBranch(const ValuePtr &condition, llvm::BasicBlock *then, llvm::BasicBlock *else_)
{
    m_Builder.CreateCondBr(condition->Load(*this), then, else_);
}

llvm::BasicBlock *llove::Builder::GetInsertBlock() const
{
    return m_Builder.GetInsertBlock();
}

void llove::Builder::SetInsertPoint(llvm::BasicBlock *block)
{
    m_Builder.SetInsertPoint(block);
}

void llove::Builder::SetInsertPointPastAllocas(llvm::Function *parent)
{
    m_Builder.SetInsertPointPastAllocas(parent);
}

void llove::Builder::ClearInsertPoint()
{
    m_Builder.ClearInsertionPoint();
}

llvm::Function *llove::Builder::GetParent() const
{
    if (const auto insert_block = m_Builder.GetInsertBlock())
        return insert_block->getParent();
    return nullptr;
}

llvm::Function *llove::Builder::GetOrCreateFunction(
    const std::string &name,
    const FunctionType::Ptr &type,
    const bool external)
{
    if (const auto function = m_Module.getFunction(name))
        return function;
    return llvm::Function::Create(
        type->Gen(*this),
        external ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
        name,
        m_Module);
}

llvm::BasicBlock *llove::Builder::CreateBlock(const std::string &name, llvm::Function *parent)
{
    return llvm::BasicBlock::Create(m_Context, name, parent);
}

void llove::Builder::AddFunction(const bool expose, std::string name, FunctionType::Ptr type, llvm::Function *callee)
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Type != type)
            continue;
        return;
    }

    m_Functions.emplace_back(
        FunctionReference
        {
            .Expose = expose,
            .Name = std::move(name),
            .Type = std::move(type),
            .Callee = callee,
        }
    );
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(const std::string &name)
{
    std::vector<FunctionReference> functions;
    for (auto &function : m_Functions)
        if (function.Name == name)
            functions.emplace_back(function);
    return functions;
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(const std::string &name, const Field &self)
{
    std::vector<FunctionReference> functions;
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (!function.Type->HasSelf())
            continue;
        auto &function_self = function.Type->GetSelf();
        if (function_self.Type != self.Type)
            continue;
        if (function_self.Mutable && !self.Mutable)
            continue;
        functions.emplace_back(function);
    }
    return functions;
}

llove::Operator<1>::Ptr llove::Builder::GetOperator(const std::string &operator_, const Field &operand, bool suffix)
{
    auto lowest_error = ~0u;
    Operator<1>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        if (function.Name != operator_)
            continue;
        if (suffix != function.Type->IsVarArg())
            continue;

        auto error = 0u;

        if (function.Type->HasSelf())
        {
            if (function.Type->GetParameterCount() != 0)
                continue;

            auto &self = function.Type->GetSelf();
            if (self.Type != operand.Type)
                continue;
            if (self.Mutable && (!operand.Reference || !operand.Mutable))
                continue;

            if (!operand.Reference)
                error += 10u;
        }
        else
        {
            if (function.Type->GetParameterCount() != 1)
                continue;

            if (auto &[mutable_, reference_, type_] = function.Type->GetParameter(0); reference_)
            {
                if (type_ != operand.Type)
                    continue;
                if (mutable_ && (!operand.Reference || !operand.Mutable))
                    continue;

                if (!operand.Reference)
                    error += 10u;
            }
            else if (type_ != operand.Type)
            {
                if (!IsCastable(operand.Mutable, operand.Type, type_))
                    continue;

                error += 10u;
            }
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<1>>(function.Type, function.Callee);
    }

    if (candidate)
        return candidate;

    if (BIUnOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<1>>(BIUnOperatorCallees.at(operator_), suffix);

    return nullptr;
}

llove::Operator<2>::Ptr llove::Builder::GetOperator(
    const std::string &operator_,
    const Field &left,
    const Field &right)
{
    auto lowest_error = ~0u;
    Operator<2>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        if (function.Name != operator_)
            continue;
        if (function.Type->IsVarArg())
            continue;

        auto error = 0u;

        if (function.Type->HasSelf())
        {
            if (function.Type->GetParameterCount() != 1)
                continue;

            auto &self = function.Type->GetSelf();
            if (self.Type != left.Type)
                continue;
            if (self.Mutable && (!left.Reference || !left.Mutable))
                continue;

            if (!left.Reference)
                error += 10u;

            if (auto &[mutable_, reference_, type_] = function.Type->GetParameter(0); reference_)
            {
                if (type_ != right.Type)
                    continue;
                if (mutable_ && (!right.Reference || !right.Mutable))
                    continue;

                if (!right.Reference)
                    error += 10u;
            }
            else if (type_ != right.Type)
            {
                if (!IsCastable(right.Mutable, right.Type, type_))
                    continue;

                error += 10u;
            }
        }
        else
        {
            if (function.Type->GetParameterCount() != 2)
                continue;

            if (auto &[mutable_, reference_, type_] = function.Type->GetParameter(0); reference_)
            {
                if (type_ != left.Type)
                    continue;
                if (mutable_ && (!left.Reference || !left.Mutable))
                    continue;

                if (!left.Reference)
                    error += 10u;
            }
            else if (type_ != left.Type)
            {
                if (!IsCastable(left.Mutable, left.Type, type_))
                    continue;

                error += 10u;
            }

            if (auto &[mutable_, reference_, type_] = function.Type->GetParameter(1); reference_)
            {
                if (type_ != right.Type)
                    continue;
                if (mutable_ && (!right.Reference || !right.Mutable))
                    continue;

                if (!right.Reference)
                    error += 10u;
            }
            else if (type_ != right.Type)
            {
                if (!IsCastable(right.Mutable, right.Type, type_))
                    continue;

                error += 10u;
            }
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<2>>(function.Type, function.Callee);
    }

    if (candidate)
        return candidate;

    if (BIBiOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<2>>(BIBiOperatorCallees.at(operator_));

    return nullptr;
}

void llove::Builder::StackPush(const Field &result)
{
    m_Stack.emplace_back(result.Type ? result : m_Stack.empty() ? Field{} : m_Stack.back().Result);
}

void llove::Builder::StackPop()
{
    // TODO: append automatic delete calls for orphan values
    m_Stack.pop_back();
}

void llove::Builder::SetValue(const std::string &name, ValuePtr value)
{
    m_Stack.back().Values[name] = std::move(value);
}

llove::ValuePtr llove::Builder::GetValue(const std::string &name) const
{
    if (m_Stack.empty())
        return nullptr;
    for (auto &[result_, values_] : std::ranges::reverse_view(m_Stack))
        if (values_.contains(name))
            return values_.at(name);
    return nullptr;
}

llove::Field llove::Builder::GetResult()
{
    if (m_Stack.empty())
        return {};
    return m_Stack.back().Result;
}

llove::ValuePtr llove::Builder::CreateCast(ValuePtr value, TypePtr type)
{
    const auto value_type = value->GetType();
    if (value_type == type)
        return value;

    const auto llvm_value = value->Load(*this);
    const auto llvm_type = type->Gen(*this);

    llvm::Value *result = nullptr;

    switch (value_type->GetId())
    {
    case TypeId_Integer:
        switch (type->GetId())
        {
        case TypeId_Integer:
            result = m_Builder.CreateIntCast(
                llvm_value,
                llvm_type,
                As<IntegerType>(type)->IsSigned());
            break;
        case TypeId_Float:
            if (As<IntegerType>(value_type)->IsSigned())
                result = m_Builder.CreateSIToFP(llvm_value, llvm_type);
            else
                result = m_Builder.CreateUIToFP(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Float:
        switch (type->GetId())
        {
        case TypeId_Integer:
            if (As<IntegerType>(type)->IsSigned())
                result = m_Builder.CreateFPToSI(llvm_value, llvm_type);
            else
                result = m_Builder.CreateFPToUI(llvm_value, llvm_type);
            break;
        case TypeId_Float:
            result = m_Builder.CreateFPCast(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Pointer:
        switch (type->GetId())
        {
        case TypeId_Integer:
            result = m_Builder.CreatePtrToInt(llvm_value, llvm_type);
            break;
        case TypeId_Pointer:
            if (As<PointerType>(value_type)->IsMutable() || !As<PointerType>(type)->IsMutable())
                result = m_Builder.CreatePointerCast(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Array:
        switch (type->GetId())
        {
        case TypeId_Pointer:
            if (As<ArrayType>(value_type)->GetBase() == As<PointerType>(type)->GetBase() &&
                (value->IsMutable() || !As<PointerType>(type)->IsMutable()))
                result = value->GetPointer();
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    Assert(result != nullptr, "cast from value of type {} to type {} not implemented", value->GetType(), type);
    return Value::CreateR(std::move(type), result);
}

bool llove::Builder::IsCastable(const bool mutable_, const TypePtr &value_type, const TypePtr &type)
{
    if (value_type == type)
        return true;

    switch (value_type->GetId())
    {
    case TypeId_Integer:
        switch (type->GetId())
        {
        case TypeId_Integer:
        case TypeId_Float:
            return true;
        default:
            return false;
        }

    case TypeId_Float:
        switch (type->GetId())
        {
        case TypeId_Integer:
        case TypeId_Float:
            return true;
        default:
            return false;
        }

    case TypeId_Pointer:
        switch (type->GetId())
        {
        case TypeId_Integer:
            return true;
        case TypeId_Pointer:
            return As<PointerType>(value_type)->IsMutable() || !As<PointerType>(type)->IsMutable();
        default:
            return false;
        }

    case TypeId_Array:
        switch (type->GetId())
        {
        case TypeId_Pointer:
            return As<ArrayType>(value_type)->GetBase() == As<PointerType>(type)->GetBase() &&
                   (mutable_ || !As<PointerType>(type)->IsMutable());
        default:
            return false;
        }

    default:
        return false;
    }
}

llvm::Value *llove::Builder::CreateGlobalString(const std::string &value)
{
    return m_Builder.CreateGlobalStringPtr(value, {}, 0, &m_Module);
}

llvm::FunctionCallee llove::Builder::GenFunction(const GenericFunction &fn)
{
    const auto mangled = Mangle(fn.Interface, fn.ClassName, fn.Mutable, fn.Name, fn.Parameters, fn.VarArg, fn.Result);

    std::vector<Field> type_parameters;
    for (auto &[info_, name_] : fn.Parameters)
        type_parameters.emplace_back(info_);

    Field self;
    FunctionType::Ptr function_type;

    if (fn.ClassName.empty())
    {
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result);
    }
    else
    {
        self = {
            .Mutable = fn.Mutable,
            .Type = m_Types.GetClass(fn.ClassName),
        };
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result, self);
    }

    const auto function = GetOrCreateFunction(mangled, function_type, fn.Interface);

    AddFunction(fn.Expose, fn.Name, function_type, function);

    if (!fn.Content)
        return { function_type->Gen(*this), function };

    const auto entry_block = CreateBlock("entry", function);

    SetInsertPoint(entry_block);
    StackPush(fn.Result);

    GenParameters(function, fn.Parameters, self);

    fn.Content->Gen(*this);

    StackPop();
    ClearInsertPoint();

    for (auto &block : *function)
    {
        if (block.getTerminator())
            continue;
        if (fn.Result.Type->GetId() == TypeId_Void)
        {
            SetInsertPoint(&block);
            m_Builder.CreateRetVoid();
            ClearInsertPoint();
            continue;
        }
        Error("not all paths yield");
    }

    const auto error = verifyFunction(*function, &llvm::errs());
    Assert(!error, "function has errors");

    return { function_type->Gen(*this), function };
}

void llove::Builder::GenParameters(llvm::Function *parent, const std::vector<Parameter> &parameters, const Field &self)
{
    auto offset = 0u;
    if (self.Type)
    {
        offset = 1u;

        const auto argument = parent->getArg(0);
        argument->setName("self");

        SetValue("self", Value::CreateL(self.Type, argument, self.Mutable));
    }

    for (unsigned i = 0; i < parent->arg_size() - offset; ++i)
    {
        auto &[info_, name_] = parameters.at(i);

        const auto argument = parent->getArg(i + offset);
        argument->setName(name_);

        llvm::Value *pointer;
        if (info_.Reference)
        {
            pointer = argument;
        }
        else
        {
            pointer = CreateAlloca(parent, info_.Type);
            CreateStore(pointer, argument);
        }

        SetValue(name_, Value::CreateL(info_.Type, pointer, info_.Mutable));
    }
}

void llove::Builder::Gen(const std::string &filename)
{
    m_Module.print(llvm::outs(), nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string target_error;

    auto target_triple = llvm::sys::getDefaultTargetTriple();
    const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);

    Assert(target != nullptr, "failed to get target for triple '{}': {}", target_triple, target_error);

    const std::string cpu = "generic";
    const std::string features;
    const llvm::TargetOptions options;

    const auto target_machine = target->createTargetMachine(target_triple, cpu, features, options, llvm::Reloc::PIC_);

    m_Module.setDataLayout(target_machine->createDataLayout());
    m_Module.setTargetTriple(target_triple);

    std::error_code error_code;
    llvm::raw_fd_ostream stream(filename, error_code, llvm::sys::fs::OF_None);

    Assert(!error_code, "failed to open file '{}': {}", filename, error_code.message());

    llvm::legacy::PassManager pass_manager;
    const auto emit_error = target_machine->addPassesToEmitFile(
        pass_manager,
        stream,
        nullptr,
        llvm::CodeGenFileType::ObjectFile);
    Assert(!emit_error, "target machine cannot emit specified file type");

    pass_manager.run(m_Module);
    stream.flush();
    stream.close();
}
