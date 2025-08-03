#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llvm::BasicBlock *llove::Builder::GetInsertBlock() const
{
    return m_Builder.GetInsertBlock();
}

void llove::Builder::SetCurrentDebugLocation(llvm::DebugLoc loc)
{
    m_Builder.SetCurrentDebugLocation(std::move(loc));
}

llvm::Value *llove::Builder::CreateAlloca(const TypePtr &type, llvm::Function *parent)
{
    const auto insert_block = m_Builder.GetInsertBlock();
    m_Builder.SetInsertPointPastAllocas(parent ? parent : insert_block->getParent());
    const auto pointer = m_Builder.CreateAlloca(type->GenIR(*this));
    m_Builder.SetInsertPoint(insert_block);
    return pointer;
}

llvm::Value *llove::Builder::CreateLoad(llvm::Value *pointer, const TypePtr &type)
{
    return m_Builder.CreateLoad(type->GenIR(*this), pointer);
}

llvm::Value *llove::Builder::CreateStore(llvm::Value *pointer, llvm::Value *value, const bool volatile_)
{
    return m_Builder.CreateStore(value, pointer, volatile_);
}

llvm::Value *llove::Builder::CreateStore(llvm::Value *pointer, const ValuePtr &value, const bool volatile_)
{
    return m_Builder.CreateStore(value->Load(*this), pointer, volatile_);
}

void llove::Builder::CreateRetVoid()
{
    m_Builder.CreateRetVoid();
}

void llove::Builder::CreateRet(llvm::Value *value)
{
    m_Builder.CreateRet(value);
}

llove::ValuePtr llove::Builder::CreateCall(
    const FunctionReference &function,
    std::vector<ValuePtr> arguments,
    ValuePtr self)
{
    auto &function_type = function.Type;
    auto &function_self = function_type->GetSelf();
    auto &function_result = function_type->GetResult();

    Assert(!self == !function_self, "illegal function call, function self does not match self");

    std::vector<llvm::Value *> argument_values;

    if (self)
    {
        argument_values.emplace_back(function_self->GenCast(*this, std::move(self)));
    }

    unsigned i;
    for (i = 0; i < function_type->GetParameterCount(); ++i)
    {
        auto &parameter = function_type->GetParameter(i);
        auto &argument = arguments.at(i);

        argument_values.emplace_back(parameter.GenCast(*this, std::move(argument)));
    }
    for (; i < arguments.size(); ++i)
    {
        argument_values.emplace_back(arguments.at(i)->Load(*this));
    }

    const auto result_value = m_Builder.CreateCall(
        function_type->GenFunction(*this),
        function.Callee,
        argument_values);

    if (function_result.Reference)
        return Value::CreateL(function_result.Type, result_value, function_result.Mutable);

    return Value::CreateR(function_result.Type, result_value);
}

llove::ValuePtr llove::Builder::CreateCall(const ValuePtr &callee)
{
    const auto function_type = As<FunctionType>(callee->GetType());
    auto &function_result = function_type->GetResult();

    const auto result_value = m_Builder.CreateCall(function_type->GenFunction(*this), callee->Load(*this));

    if (function_result.Reference)
        return Value::CreateL(function_result.Type, result_value, function_result.Mutable);

    return Value::CreateR(function_result.Type, result_value);
}

llvm::Value *llove::Builder::CreateInsertValue(llvm::Value *aggregate, llvm::Value *value, const unsigned index)
{
    return m_Builder.CreateInsertValue(aggregate, value, index);
}

llvm::Value *llove::Builder::CreateExtractValue(llvm::Value *aggregate, const unsigned index)
{
    return m_Builder.CreateExtractValue(aggregate, index);
}

llvm::Value *llove::Builder::CreateExtractValue(const ValuePtr &aggregate, const unsigned index)
{
    return m_Builder.CreateExtractValue(aggregate->Load(*this), index);
}

llvm::Value *llove::Builder::CreatePointerOffset(llvm::Type *element_type, llvm::Value *pointer, const unsigned offset)
{
    return m_Builder.CreateConstGEP1_64(element_type, pointer, offset);
}

llove::ValuePtr llove::Builder::CreatePointerOffset(const ValuePtr &pointer, const unsigned offset)
{
    auto type = As<PointerType>(pointer->GetType());
    const auto element_pointer = m_Builder.CreateConstGEP1_64(
        type->GetBase()->GenIR(*this),
        pointer->Load(*this),
        offset);
    return Value::CreateR(std::move(type), element_pointer);
}

llove::ValuePtr llove::Builder::CreatePointerOffset(const ValuePtr &pointer, const ValuePtr &offset)
{
    auto type = As<PointerType>(pointer->GetType());
    const auto element_pointer = m_Builder.CreateGEP(
        type->GetBase()->GenIR(*this),
        pointer->Load(*this),
        offset->Load(*this));
    return Value::CreateR(std::move(type), element_pointer);
}

llove::ValuePtr llove::Builder::CreatePointerDifference(const ValuePtr &begin, const ValuePtr &end)
{
    const auto type = As<PointerType>(begin->GetType());
    const auto value = m_Builder.CreatePtrDiff(
        type->GetBase()->GenIR(*this),
        begin->Load(*this),
        end->Load(*this));
    return Value::CreateR(m_Types.GetInteger(true, 64), value);
}

llove::ValuePtr llove::Builder::CreatePointerElement(const ValuePtr &pointer, const ValuePtr &index)
{
    const auto type = As<PointerType>(pointer->GetType());
    const auto value = m_Builder.CreateGEP(
        type->GetBase()->GenIR(*this),
        pointer->Load(*this),
        index->Load(*this));
    return Value::CreateL(type->GetBase(), value, type->IsMutable());
}

llove::ValuePtr llove::Builder::CreateArrayElement(const ValuePtr &array, const ValuePtr &index)
{
    const auto type = As<ArrayType>(array->GetType());
    const auto base_type = type->GetBase();

    const auto index_value = index->Load(*this);

    if (array->IsReferenceable())
    {
        const auto element_pointer = m_Builder.CreateGEP(base_type->GenIR(*this), array->GetPointer(), index_value);
        return Value::CreateL(base_type, element_pointer, array->IsMutable());
    }

    if (const auto const_index_value = llvm::dyn_cast<llvm::ConstantInt>(index_value))
    {
        const auto value = m_Builder.CreateExtractValue(array->Load(*this), const_index_value->getLimitedValue());
        return Value::CreateR(base_type, value);
    }

    const auto pointer = CreateAlloca(type);
    CreateStore(pointer, array);

    const auto element_pointer = m_Builder.CreateGEP(base_type->GenIR(*this), pointer, index_value);
    return Value::CreateL(base_type, element_pointer, false);
}

llvm::Value *llove::Builder::CreateArrayGEP(const TypePtr &type, llvm::Value *pointer, const unsigned index)
{
    return m_Builder.CreateConstGEP2_64(type->GenIR(*this), pointer, 0, index);
}

llvm::Value *llove::Builder::CreateStructGEP(const TypePtr &type, llvm::Value *pointer, const unsigned index)
{
    return m_Builder.CreateStructGEP(type->GenIR(*this), pointer, index);
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

llvm::Value *llove::Builder::CreatePCmpEQ(llvm::Value *left, llvm::Value *right)
{
    const auto int_type = GetIntType(64);
    const auto left_int = m_Builder.CreatePtrToInt(left, int_type);
    const auto right_int = m_Builder.CreatePtrToInt(right, int_type);
    return m_Builder.CreateICmpEQ(left_int, right_int);
}

llove::ValuePtr llove::Builder::CreatePCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = CreatePCmpEQ(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
}

llvm::Value *llove::Builder::CreatePCmpNE(llvm::Value *left, llvm::Value *right)
{
    const auto int_type = GetIntType(64);
    const auto left_int = m_Builder.CreatePtrToInt(left, int_type);
    const auto right_int = m_Builder.CreatePtrToInt(right, int_type);
    return m_Builder.CreateICmpNE(left_int, right_int);
}

llove::ValuePtr llove::Builder::CreatePCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = CreatePCmpNE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Types.GetInteger(false, 1), value);
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

void llove::Builder::CreateBranch(llvm::Value *condition, llvm::BasicBlock *then, llvm::BasicBlock *else_)
{
    m_Builder.CreateCondBr(condition, then, else_);
}

void llove::Builder::CreateBranch(const ValuePtr &condition, llvm::BasicBlock *then, llvm::BasicBlock *else_)
{
    m_Builder.CreateCondBr(condition->Load(*this), then, else_);
}

void llove::Builder::SetInsertPoint(llvm::BasicBlock *block)
{
    m_Builder.SetInsertPoint(block);
}

void llove::Builder::ClearInsertPoint()
{
    m_Builder.ClearInsertionPoint();
}

bool llove::Builder::NoTerminator() const
{
    return m_Builder.GetInsertBlock()->getTerminator() == nullptr;
}

llvm::BasicBlock *llove::Builder::CreateBlock(const std::string &name, llvm::Function *parent)
{
    return llvm::BasicBlock::Create(m_Context, name, parent);
}
