#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::CreateCast(ValuePtr value, TypePtr dst)
{
    const auto src_fld = value->AsField();
    const auto dst_fld = Field{ .Type = dst };

    const auto src = value->GetType();
    if (src == dst)
        return value;

    const FunctionReference *callee = nullptr;

    for (auto &function : m_Functions)
    {
        if (function.Name != "cast")
            continue;

        const auto function_type = function.Type;
        if (function_type->IsVarArg())
            continue;

        if (function_type->HasSelf())
        {
            if (function_type->GetParameterCount() != 0)
                continue;
            if (!Field::IsAssignable(dst_fld, function_type->GetResult()))
                continue;
            if (!Field::IsAssignable(function_type->GetSelf(), src_fld))
                continue;
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
                continue;
            if (!Field::IsAssignable(dst_fld, function_type->GetResult()))
                continue;
            if (!Field::IsAssignable(function_type->GetParameter(0), src_fld))
                continue;
        }

        callee = &function;
        break;
    }

    if (callee)
    {
        auto argument = callee->Type->HasSelf()
                            ? callee->Type->GetSelf().GenCast(*this, value)
                            : callee->Type->GetParameter(0).GenCast(*this, value);

        const auto result = CreateCall(callee->Type, callee->Callee, { argument });
        return Value::CreateR(dst, result);
    }

    const auto llvm_value = value->Load(*this);
    const auto llvm_type = dst->Gen(*this);

    llvm::Value *result = nullptr;

    switch (src->GetId())
    {
    case TypeId_Integer:
        switch (dst->GetId())
        {
        case TypeId_Integer:
            result = m_Builder.CreateIntCast(
                llvm_value,
                llvm_type,
                As<IntegerType>(dst)->IsSigned());
            break;
        case TypeId_Float:
            if (As<IntegerType>(src)->IsSigned())
                result = m_Builder.CreateSIToFP(llvm_value, llvm_type);
            else
                result = m_Builder.CreateUIToFP(llvm_value, llvm_type);
            break;
        default:
            break;
        }
        break;

    case TypeId_Float:
        switch (dst->GetId())
        {
        case TypeId_Integer:
            if (As<IntegerType>(dst)->IsSigned())
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
        switch (dst->GetId())
        {
        case TypeId_Integer:
            result = m_Builder.CreatePtrToInt(llvm_value, llvm_type);
            break;
        case TypeId_Pointer:
            if (As<PointerType>(src)->IsMutable() || !As<PointerType>(dst)->IsMutable())
                result = m_Builder.CreatePointerCast(llvm_value, llvm_type);
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

    Assert(result != nullptr, "cast from value of type {} to type {} not implemented", value->GetType(), dst);
    return Value::CreateR(std::move(dst), result);
}

bool llove::Builder::IsCastable(const Field &src, const Field &dst) const
{
    if (src == dst)
        return true;

    for (auto &function : m_Functions)
    {
        if (function.Name != "cast")
            continue;

        const auto function_type = function.Type;
        if (function_type->IsVarArg())
            continue;

        auto &res = function_type->GetResult();

        if (function_type->HasSelf())
        {
            if (function_type->GetParameterCount() != 0)
                continue;
            if (!Field::IsAssignable(dst, res))
                continue;
            if (!Field::IsAssignable(function_type->GetSelf(), src))
                continue;
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
                continue;
            if (!Field::IsAssignable(dst, res))
                continue;
            if (!Field::IsAssignable(function_type->GetParameter(0), src))
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
