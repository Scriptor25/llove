#include <llove/type.hpp>

unsigned llove::Difference(
    const TypePtr& left,
    const TypePtr& right)
{
    if (left == right)
        return 0u;

    switch (left->GetId())
    {
    case TypeId_Integer:
    {
        const auto left_int = As<IntegerType>(left);
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto right_int = As<IntegerType>(right);
            const auto sign_error = left_int->IsSigned() != right_int->IsSigned() ? 1u : 0u;
            const auto bits_error = left_int->GetBits() != right_int->GetBits() ? 5u : 0u;
            return sign_error + bits_error;
        }
        case TypeId_Float:
        {
            const auto right_flt = As<FloatType>(right);
            const auto bits_error = left_int->GetBits() != right_flt->GetBits() ? 5u : 0u;
            return 5u + bits_error;
        }
        default:
            break;
        }
        break;
    }
    case TypeId_Float:
    {
        const auto left_flt = As<FloatType>(left);
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto right_int = As<IntegerType>(right);
            const auto bits_error = left_flt->GetBits() != right_int->GetBits() ? 5u : 0u;
            return 5u + bits_error;
        }
        case TypeId_Float:
        {
            const auto right_flt = As<FloatType>(right);
            const auto bits_error = left_flt->GetBits() != right_flt->GetBits() ? 5u : 0u;
            return bits_error;
        }
        default:
            break;
        }
        break;
    }
    case TypeId_Pointer:
    {
        const auto left_ptr = As<PointerType>(left);
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto right_int = As<IntegerType>(right);
            const auto sign_error = false != right_int->IsSigned() ? 1u : 0u;
            const auto bits_error = 64u != right_int->GetBits() ? 5u : 0u; // TODO:
                                                                           // target
                                                                           // dependent
            return 4u + sign_error + bits_error;
        }
        case TypeId_Pointer:
        {
            const auto right_ptr = As<PointerType>(right);
            const auto opaque_error = left_ptr->IsOpaque() != right_ptr->IsOpaque() ? 2u : 0u;
            const auto left_base = left_ptr->IsOpaque() ? nullptr : left_ptr->GetBase();
            const auto right_base = right_ptr->IsOpaque() ? nullptr : right_ptr->GetBase();
            const auto type_error = left_base != right_base ? 2u : 0u;
            const auto mut_error = left_ptr->IsMutable() != right_ptr->IsMutable() ? 1u : 0u;
            return opaque_error + type_error + mut_error;
        }
        default:
            break;
        }
        break;
    }
    case TypeId_Array:
    {
        const auto left_arr = As<ArrayType>(left);
        switch (right->GetId())
        {
        case TypeId_Pointer:
        {
            const auto right_ptr = As<PointerType>(right);
            const auto opaque_error = false != right_ptr->IsOpaque() ? 2u : 0u;
            const auto right_base = right_ptr->IsOpaque() ? nullptr : right_ptr->GetBase();
            const auto type_error = left_arr->GetBase() != right_base ? 2u : 0u;
            return 6u + opaque_error + type_error;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return 10u;
}
