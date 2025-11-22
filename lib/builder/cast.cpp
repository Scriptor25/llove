#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::CreateCast(
    ValuePtr value,
    TypePtr dst,
    const bool implicit)
{
    const auto src_fld(value->AsField());
    const Field dst_fld(dst);

    const auto src = value->GetType();
    if (src == dst)
    {
        return value;
    }

    std::optional<FunctionReference> callee;
    for (auto& function : m_Functions)
    {
        const auto& function_type = function.Type;
        const auto& function_result = function_type->GetResult();

        if (function.Name != "cast")
        {
            continue;
        }

        if (implicit && !function.IsImplicit)
        {
            continue;
        }

        if (function_type->HasVariadic())
        {
            continue;
        }

        if (const auto& function_self = function_type->GetSelf())
        {
            if (function_type->GetParameterCount() != 0)
            {
                continue;
            }
            if (!Field::IsCastable(*this, dst_fld, function_result, true))
            {
                continue;
            }
            if (!Field::IsCastable(*this, *function_self, src_fld, true))
            {
                continue;
            }
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
            {
                continue;
            }
            if (!Field::IsCastable(*this, dst_fld, function_result, true))
            {
                continue;
            }
            if (!Field::IsCastable(*this, function_type->GetParameter(0), src_fld, true))
            {
                continue;
            }
        }

        callee = function;
        break;
    }

    if (callee)
    {
        std::vector<ValuePtr> arguments;
        ValuePtr self;

        if (callee->Type->GetSelf())
        {
            self = std::move(value);
        }
        else
        {
            arguments.emplace_back(std::move(value));
        }

        return CreateCall(*callee, std::move(arguments), std::move(self));
    }

    auto src_llvm = src->GenIR(*this);
    auto dst_llvm = dst->GenIR(*this);

    llvm::Value* result = nullptr;

    switch (src->GetId())
    {
    case TypeId_Integer:
    {
        const auto src_integer = As<IntegerType>(src);
        switch (dst->GetId())
        {
        case TypeId_Integer:
        {
            const auto dst_integer = As<IntegerType>(dst);
            if (dst_integer->GetBits() == 1)
            {
                result = m_LLVMBuilder.CreateIsNotNull(value->Load(*this));
                break;
            }
            result = m_LLVMBuilder.CreateIntCast(
                value->Load(*this),
                dst_llvm,
                dst_integer->IsSigned());
            break;
        }
        case TypeId_Float:
            if (src_integer->IsSigned())
            {
                result = m_LLVMBuilder.CreateSIToFP(value->Load(*this), dst_llvm);
                break;
            }
            result = m_LLVMBuilder.CreateUIToFP(value->Load(*this), dst_llvm);
            break;
        default:
            break;
        }
        break;
    }

    case TypeId_Float:
        switch (dst->GetId())
        {
        case TypeId_Integer:
        {
            const auto dst_integer = As<IntegerType>(dst);
            if (dst_integer->GetBits() == 1)
            {
                result = m_LLVMBuilder.CreateIsNotNull(value->Load(*this));
                break;
            }
            if (dst_integer->IsSigned())
            {
                result = m_LLVMBuilder.CreateFPToSI(value->Load(*this), dst_llvm);
                break;
            }
            result = m_LLVMBuilder.CreateFPToUI(value->Load(*this), dst_llvm);
            break;
        }
        case TypeId_Float:
            result = m_LLVMBuilder.CreateFPCast(value->Load(*this), dst_llvm);
            break;
        default:
            break;
        }
        break;

    case TypeId_Pointer:
    {
        const auto src_pointer = As<PointerType>(src);
        switch (dst->GetId())
        {
        case TypeId_Integer:
            if (const auto dst_integer = As<IntegerType>(dst); dst_integer->GetBits() == 1)
            {
                result = m_LLVMBuilder.CreateIsNotNull(value->Load(*this));
                break;
            }
            result = m_LLVMBuilder.CreatePtrToInt(value->Load(*this), dst_llvm);
            break;
        case TypeId_Pointer:
            if (const auto dst_pointer = As<PointerType>(dst);
                !src_pointer->IsMutable() && dst_pointer->IsMutable())
            {
                break;
            }
            result = value->Load(*this);
            break;
        default:
            break;
        }
        break;
    }

    case TypeId_Array:
    {
        const auto src_array = As<ArrayType>(src);
        switch (dst->GetId())
        {
        case TypeId_Pointer:
        {
            const auto dst_pointer = As<PointerType>(dst);
            if (!dst_pointer->IsOpaque() && src_array->GetBase() != dst_pointer->GetBase())
            {
                break;
            }
            if (!value->IsMutable() && dst_pointer->IsMutable())
            {
                break;
            }
            result = value->GetPointer();
            break;
        }
        default:
            break;
        }
        break;
    }

    case TypeId_Class:
        if (As<ClassType>(src)->InheritsFrom(dst))
        {
            llvm::Value* pointer;
            if (value->IsReference())
            {
                pointer = value->GetPointer();
            }
            else
            {
                pointer = CreateAlloca(src_llvm);
                CreateStore(value->Load(*this), pointer);
            }
            result = CreateLoad(dst_llvm, pointer);
        }
        break;

    case TypeId_Function:
        switch (dst->GetId())
        {
        case TypeId_Function:
            if (implicit)
            {
                break;
            }
            result = value->Load(*this);
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

bool llove::Builder::IsCastable(
    const Field& src,
    const Field& dst,
    const bool implicit) const
{
    if (src == dst)
    {
        return true;
    }

    for (auto& function : m_Functions)
    {
        if (function.Name != "cast")
        {
            continue;
        }

        if (implicit && !function.IsImplicit)
        {
            continue;
        }

        const auto& function_type = function.Type;
        const auto& function_result = function_type->GetResult();

        if (function_type->HasVariadic())
        {
            continue;
        }

        if (const auto& function_self = function_type->GetSelf())
        {
            if (function_type->GetParameterCount() != 0)
            {
                continue;
            }
            if (!Field::IsCastable(*this, dst, function_result, true))
            {
                continue;
            }
            if (!Field::IsCastable(*this, *function_self, src, true))
            {
                continue;
            }
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
            {
                continue;
            }
            if (!Field::IsCastable(*this, dst, function_result, true))
            {
                continue;
            }
            if (!Field::IsCastable(*this, function_type->GetParameter(0), src, true))
            {
                continue;
            }
        }
        return true;
    }

    const auto src_type = src.GetType();
    const auto dst_type = dst.GetType();

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
    {
        const auto src_pointer = As<PointerType>(src_type);
        switch (dst_type->GetId())
        {
        case TypeId_Integer:
            return true;
        case TypeId_Pointer:
        {
            const auto dst_pointer = As<PointerType>(dst_type);
            return src_pointer->IsMutable() || !dst_pointer->IsMutable();
        }
        default:
            return false;
        }
    }

    case TypeId_Array:
    {
        const auto src_array = As<ArrayType>(src_type);
        switch (dst_type->GetId())
        {
        case TypeId_Pointer:
        {
            const auto dst_pointer = As<PointerType>(dst_type);
            return (dst_pointer->IsOpaque()
                    || src_array->GetBase() == dst_pointer->GetBase())
                && (src.IsMutable() || !dst_pointer->IsMutable());
        }
        default:
            return false;
        }
    }

    case TypeId_Class:
        return As<ClassType>(src_type)->InheritsFrom(dst_type);

    case TypeId_Function:
        switch (dst_type->GetId())
        {
        case TypeId_Function:
            return !implicit;
        default:
            return false;
        }

    default:
        return false;
    }
}
