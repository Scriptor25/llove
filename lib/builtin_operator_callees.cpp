#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/operator.hpp>
#include <llove/value.hpp>

static llove::ValuePtr operator_neg(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    switch (operand->GetType()->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateNeg(operand);
    case llove::TypeId_Float:
        return builder.CreateFNeg(operand);
    default:
        break;
    }

    llove::Error("operator '-{}' not implemented", operand->AsField());
}

static llove::ValuePtr operator_not(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    switch (operand->GetType()->GetId())
    {
    case llove::TypeId_Integer:
    case llove::TypeId_Float:
    case llove::TypeId_Pointer:
        return builder.CreateNot(operand);
    default:
        break;
    }

    llove::Error("operator '!{}' not implemented", operand->AsField());
}

static llove::ValuePtr operator_inv(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    switch (operand->GetType()->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateInv(operand);
    default:
        break;
    }

    llove::Error("operator '~{}' not implemented", operand->AsField());
}

static llove::ValuePtr operator_inc(llove::Builder &builder, llove::ValuePtr operand, const bool suffix)
{
    const auto pre = suffix ? operand->Load(builder) : nullptr;

    llove::ValuePtr result;
    switch (operand->GetType()->GetId())
    {
    case llove::TypeId_Integer:
    {
        const auto one_value = llvm::ConstantInt::get(operand->GetType()->GenIR(builder), 1, false);
        const auto one = llove::Value::CreateR(operand->GetType(), one_value);
        result = builder.CreateAdd(operand, one);
        break;
    }
    case llove::TypeId_Float:
    {
        const auto one_value = llvm::ConstantFP::get(operand->GetType()->GenIR(builder), 1.0);
        const auto one = llove::Value::CreateR(operand->GetType(), one_value);
        result = builder.CreateFAdd(operand, one);
        break;
    }
    case llove::TypeId_Pointer:
    {
        const auto one_value = llvm::ConstantInt::get(builder.GetIntType(64), 1, false);
        const auto one = llove::Value::CreateR(builder.GetTypes().GetInteger(false, 64), one_value);
        result = builder.CreatePointerOffset(operand, one);
        break;
    }
    default:
        llove::Error("operator '{}{}{}' not implemented", suffix ? "" : "++", operand->AsField(), suffix ? "++" : "");
    }

    operand->Store(builder, result);

    if (suffix)
        return llove::Value::CreateR(operand->GetType(), pre);

    return operand;
}

static llove::ValuePtr operator_dec(llove::Builder &builder, llove::ValuePtr operand, const bool suffix)
{
    const auto pre = suffix ? operand->Load(builder) : nullptr;

    llove::ValuePtr result;
    switch (operand->GetType()->GetId())
    {
    case llove::TypeId_Integer:
    {
        const auto one_value = llvm::ConstantInt::get(operand->GetType()->GenIR(builder), 1, false);
        const auto one = llove::Value::CreateR(operand->GetType(), one_value);
        result = builder.CreateSub(operand, one);
        break;
    }
    case llove::TypeId_Float:
    {
        const auto one_value = llvm::ConstantFP::get(operand->GetType()->GenIR(builder), 1.0);
        const auto one = llove::Value::CreateR(operand->GetType(), one_value);
        result = builder.CreateFSub(operand, one);
        break;
    }
    case llove::TypeId_Pointer:
    {
        const auto one_value = llvm::ConstantInt::get(builder.GetIntType(64), -1, false);
        const auto one = llove::Value::CreateR(builder.GetTypes().GetInteger(true, 64), one_value);
        result = builder.CreatePointerOffset(operand, one);
        break;
    }
    default:
        llove::Error("operator '{}{}{}' not implemented", suffix ? "" : "--", operand->AsField(), suffix ? "--" : "");
    }

    operand->Store(builder, result);

    if (suffix)
        return llove::Value::CreateR(operand->GetType(), pre);

    return operand;
}

static llove::ValuePtr operator_deref(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    llove::Assert(operand->GetType()->IsPointer(), "illegal dereference of non-pointer value");
    const auto type = llove::As<llove::PointerType>(operand->GetType());
    return llove::Value::CreateL(type->GetBase(), operand->Load(builder), type->IsMutable());
}

static llove::ValuePtr operator_ref(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    return operand->Reference(builder);
}

static llove::ValuePtr operator_copy(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    right = builder.CreateCast(std::move(right), left->GetType(), true);
    left->Store(builder, right);
    return left;
}

static llove::ValuePtr operator_add(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Integer)
        return builder.CreatePointerOffset(left, right);
    if (left_type->GetId() == llove::TypeId_Integer && right_type->GetId() == llove::TypeId_Pointer)
        return builder.CreatePointerOffset(right, left);

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateAdd(left, right);
    case llove::TypeId_Float:
        return builder.CreateFAdd(left, right);
    default:
        break;
    }

    llove::Error("operator '{} + {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_sub(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Integer)
    {
        const auto offset = builder.CreateNeg(right);
        return builder.CreatePointerOffset(left, offset);
    }
    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Pointer)
        return builder.CreatePointerDifference(left, right);

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateSub(left, right);
    case llove::TypeId_Float:
        return builder.CreateFSub(left, right);
    default:
        break;
    }

    llove::Error("operator '{} - {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_mul(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateMul(left, right);
    case llove::TypeId_Float:
        return builder.CreateFMul(left, right);
    default:
        break;
    }

    llove::Error("operator '{} * {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_div(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateDiv(left, right);
    case llove::TypeId_Float:
        return builder.CreateFDiv(left, right);
    default:
        break;
    }

    llove::Error("operator '{} / {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_rem(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateRem(left, right);
    case llove::TypeId_Float:
        return builder.CreateFRem(left, right);
    default:
        break;
    }

    llove::Error("operator '{} % {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_and(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateAnd(left, right);
    default:
        break;
    }

    llove::Error("operator '{} & {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_or(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateOr(left, right);
    default:
        break;
    }

    llove::Error("operator '{} | {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_xor(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateXor(left, right);
    default:
        break;
    }

    llove::Error("operator '{} ^ {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_eq(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpEQ(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpEQ(left, right);
    case llove::TypeId_Pointer:
        return builder.CreatePCmpEQ(left, right);
    default:
        break;
    }

    llove::Error("operator '{} == {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_ne(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpNE(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpNE(left, right);
    case llove::TypeId_Pointer:
        return builder.CreatePCmpNE(left, right);
    default:
        break;
    }

    llove::Error("operator '{} != {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_lt(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpLT(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpLT(left, right);
    default:
        break;
    }

    llove::Error("operator '{} < {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_gt(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpGT(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpGT(left, right);
    default:
        break;
    }

    llove::Error("operator '{} > {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_le(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpLE(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpLE(left, right);
    default:
        break;
    }

    llove::Error("operator '{} <= {}' not implemented", left->AsField(), right->AsField());
}

static llove::ValuePtr operator_ge(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetTypes().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpGE(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpGE(left, right);
    default:
        break;
    }

    llove::Error("operator '{} >= {}' not implemented", left->AsField(), right->AsField());
}

const std::map<std::string_view, llove::BIOperator<1>::CalleeType> llove::BIUnOperatorCallees
{
    { "-", operator_neg },
    { "!", operator_not },
    { "~", operator_inv },

    { "++", operator_inc },
    { "--", operator_dec },

    { "*", operator_deref },
    { "&", operator_ref },
};

const std::map<std::string_view, llove::BIOperator<2>::CalleeType> llove::BIBiOperatorCallees
{
    { "=", operator_copy },

    { "+", operator_add },
    { "-", operator_sub },
    { "*", operator_mul },
    { "/", operator_div },
    { "%", operator_rem },

    { "&", operator_and },
    { "|", operator_or },
    { "^", operator_xor },

    { "==", operator_eq },
    { "!=", operator_ne },
    { "<", operator_lt },
    { ">", operator_gt },
    { "<=", operator_le },
    { ">=", operator_ge },
};
