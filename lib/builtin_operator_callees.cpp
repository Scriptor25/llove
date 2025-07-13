#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/operator.hpp>
#include <llove/value.hpp>

static llove::ValuePtr operator_add(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    auto left_type = left->GetType();
    auto right_type = right->GetType();

    if (left_type->GetId() == llove::TypeId_Pointer && right_type->GetId() == llove::TypeId_Integer)
        return builder.CreatePointerOffset(left, right);
    if (left_type->GetId() == llove::TypeId_Integer && right_type->GetId() == llove::TypeId_Pointer)
        return builder.CreatePointerOffset(right, left);

    const auto type = builder.GetTypes().DetermineHigherOrder(std::move(left_type), std::move(right_type));
    left = builder.GenCast(std::move(left), type);
    right = builder.GenCast(std::move(right), type);

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
    left = builder.GenCast(std::move(left), type);
    right = builder.GenCast(std::move(right), type);

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
    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_div(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_rem(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    llove::Error("not yet implemented");
}

static llove::ValuePtr operator_copy(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    right = builder.GenCast(std::move(right), left->GetType());
    left->Store(builder, right->Load(builder));
    return left;
}

std::map<std::string, llove::BuiltinOperator::CalleeType> llove::BuiltinOperatorCallees
{
    { "+", operator_add },
    { "-", operator_sub },
    { "*", operator_mul },
    { "/", operator_div },
    { "%", operator_rem },

    { "=", operator_copy },
};
