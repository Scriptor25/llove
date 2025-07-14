#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/operator.hpp>
#include <llove/value.hpp>

static llove::ValuePtr operator_copy(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    right = builder.CreateCast(std::move(right), left->GetType());
    left->Store(builder, right->Load(builder));
    return left;
}

static llove::ValuePtr operator_add(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Integer)
        return builder.CreatePointerOffset(left, right);
    if (left_type->GetId() == llove::TypeId_Integer && right_type->GetId() == llove::TypeId_Pointer)
        return builder.CreatePointerOffset(right, left);

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateAdd(left, right);
    case llove::TypeId_Float:
        return builder.CreateFAdd(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_sub(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Integer)
    {
        const auto offset = builder.CreateNeg(right);
        return builder.CreatePointerOffset(left, offset);
    }
    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Pointer)
        return builder.CreatePointerDifference(left, right);

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateSub(left, right);
    case llove::TypeId_Float:
        return builder.CreateFSub(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_mul(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateMul(left, right);
    case llove::TypeId_Float:
        return builder.CreateFMul(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_div(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateDiv(left, right);
    case llove::TypeId_Float:
        return builder.CreateFDiv(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_rem(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateRem(left, right);
    case llove::TypeId_Float:
        return builder.CreateFRem(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_and(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateAnd(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_or(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateOr(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_xor(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateXor(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_eq(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

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

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_ne(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

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

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_lt(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpLT(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpLT(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_gt(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpGT(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpGT(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_le(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpLE(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpLE(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_ge(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.CreateCast(std::move(left), type);
    right = builder.CreateCast(std::move(right), type);

    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        return builder.CreateCmpGE(left, right);
    case llove::TypeId_Float:
        return builder.CreateFCmpGE(left, right);
    default:
        break;
    }

    llove::Error("not yet implemented");
}

std::map<std::string, llove::BuiltinOperator::CalleeType> llove::BuiltinOperatorCallees
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
