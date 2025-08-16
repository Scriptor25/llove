#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::CreateCast(ValuePtr value, TypePtr dst, const bool implicit)
{
    const auto src_fld = value->AsField();
    const auto dst_fld = Field{ .Type = dst };

    const auto src = value->GetType();
    if (src == dst)
        return value;

    std::optional<FunctionReference> callee;

    for (auto &function : m_Functions)
    {
        const auto &function_type = function.Type;
        const auto &function_result = function_type->GetResult();

        if (function.Name != "cast")
            continue;

        if (implicit && !function.Implicit)
            continue;

        if (function_type->IsVarArg())
            continue;

        if (const auto &function_self = function_type->GetSelf())
        {
            if (function_type->GetParameterCount() != 0)
                continue;
            if (!Field::IsCastable(*this, dst_fld, function_result, true))
                continue;
            if (!Field::IsCastable(*this, *function_self, src_fld, true))
                continue;
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
                continue;
            if (!Field::IsCastable(*this, dst_fld, function_result, true))
                continue;
            if (!Field::IsCastable(*this, function_type->GetParameter(0), src_fld, true))
                continue;
        }

        callee = function;
        break;
    }

    if (callee)
    {
        std::vector<ValuePtr> arguments;
        ValuePtr self;

        if (callee->Type->GetSelf())
            self = std::move(value);
        else
            arguments.emplace_back(std::move(value));

        return CreateCall(*callee, std::move(arguments), std::move(self));
    }

    const auto llvm_value = value->Load(*this);
    const auto llvm_type = dst->GenIR(*this);

    llvm::Value *result = nullptr;

    switch (src->GetId())
    {
    case TypeId_Integer:
        switch (dst->GetId())
        {
        case TypeId_Integer:
            if (As<IntegerType>(dst)->GetBits() == 1)
                result = m_LLVMBuilder.CreateIsNotNull(llvm_value);
            else
                result = m_LLVMBuilder.CreateIntCast(
                    llvm_value,
                    llvm_type,
                    As<IntegerType>(dst)->IsSigned());
            break;
        case TypeId_Float:
            if (As<IntegerType>(src)->IsSigned())
                result = m_LLVMBuilder.CreateSIToFP(llvm_value, llvm_type);
            else
                result = m_LLVMBuilder.CreateUIToFP(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Float:
        switch (dst->GetId())
        {
        case TypeId_Integer:
            if (As<IntegerType>(dst)->GetBits() == 1)
                result = m_LLVMBuilder.CreateIsNotNull(llvm_value);
            else if (As<IntegerType>(dst)->IsSigned())
                result = m_LLVMBuilder.CreateFPToSI(llvm_value, llvm_type);
            else
                result = m_LLVMBuilder.CreateFPToUI(llvm_value, llvm_type);
            break;
        case TypeId_Float:
            result = m_LLVMBuilder.CreateFPCast(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Pointer:
        switch (dst->GetId())
        {
        case TypeId_Integer:
            if (As<IntegerType>(dst)->GetBits() == 1)
                result = m_LLVMBuilder.CreateIsNotNull(llvm_value);
            else
                result = m_LLVMBuilder.CreatePtrToInt(llvm_value, llvm_type);
            break;
        case TypeId_Pointer:
            if (As<PointerType>(src)->IsMutable() || !As<PointerType>(dst)->IsMutable())
                result = m_LLVMBuilder.CreatePointerCast(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Array:
        switch (dst->GetId())
        {
        case TypeId_Pointer:
            if (As<ArrayType>(src)->GetBase() == As<PointerType>(dst)->GetBase() &&
                (value->IsMutable() || !As<PointerType>(dst)->IsMutable()))
                result = value->GetPointer();
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    Assert(result != nullptr, "illegal cast from '{}' to '{}'", src, dst);
    return Value::CreateR(std::move(dst), result);
}

bool llove::Builder::IsCastable(const Field &src, const Field &dst, const bool implicit) const
{
    if (src == dst)
        return true;

    for (auto &function : m_Functions)
    {
        const auto &function_type = function.Type;
        const auto &function_result = function_type->GetResult();

        if (function.Name != "cast")
            continue;

        if (implicit && !function.Implicit)
            continue;

        if (function_type->IsVarArg())
            continue;

        if (const auto &function_self = function_type->GetSelf())
        {
            if (function_type->GetParameterCount() != 0)
                continue;
            if (!Field::IsCastable(*this, dst, function_result, true))
                continue;
            if (!Field::IsCastable(*this, *function_self, src, true))
                continue;
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
                continue;
            if (!Field::IsCastable(*this, dst, function_result, true))
                continue;
            if (!Field::IsCastable(*this, function_type->GetParameter(0), src, true))
                continue;
        }
        return true;
    }

    const auto src_type = src.Type;
    const auto dst_type = dst.Type;

    switch (src_type->GetId())
    {
    case TypeId_Integer:
    case TypeId_Float:
        switch (dst_type->GetId())
        {
        case TypeId_Integer:
        case TypeId_Float:
            return true;
        default:
            return false;
        }

    case TypeId_Pointer:
        switch (dst_type->GetId())
        {
        case TypeId_Integer:
            return true;
        case TypeId_Pointer:
            return As<PointerType>(src_type)->IsMutable() || !As<PointerType>(dst_type)->IsMutable();
        default:
            return false;
        }

    case TypeId_Array:
        switch (dst_type->GetId())
        {
        case TypeId_Pointer:
            return As<ArrayType>(src_type)->GetBase() == As<PointerType>(dst_type)->GetBase() &&
                   (src.Mutable || !As<PointerType>(dst_type)->IsMutable());
        default:
            return false;
        }

    default:
        return false;
    }
}
