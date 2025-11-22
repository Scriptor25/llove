#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::TernaryExpression::TernaryExpression(
    Location loc,
    ExpressionPtr condition,
    ExpressionPtr then_,
    ExpressionPtr else_)
    : Expression(std::move(loc)),
      m_Condition(std::move(condition)),
      m_Then(std::move(then_)),
      m_Else(std::move(else_))
{
    Assert(!!m_Condition, "condition must not be null");
    Assert(!!m_Then, "then must not be null");
    Assert(!!m_Else, "else must not be null");
}

llove::ValuePtr llove::TernaryExpression::GenVal(
    Builder& builder,
    const TypePtr expect) const
try
{
    const auto parent = builder.GetParent();
    auto then_block = builder.CreateBlock("then", parent);
    auto else_block = builder.CreateBlock("else", parent);
    const auto tail_block = builder.CreateBlock("tail", parent);

    builder.EmitLoc(m_Loc);

    auto condition = m_Condition->GenVal(builder, builder.GetContext().GetBoolean());
    condition = builder.CreateCast(std::move(condition), builder.GetContext().GetBoolean(), false);
    builder.CreateBranch(condition->Load(builder), then_block, else_block);

    builder.SetInsertPoint(then_block);
    auto then_value = m_Then->GenVal(builder, expect);
    then_block = builder.GetInsertBlock();
    const auto then_branch = builder.CreateBranch(tail_block);

    builder.SetInsertPoint(else_block);
    auto else_value = m_Else->GenVal(builder, expect);
    else_block = builder.GetInsertBlock();
    const auto else_branch = builder.CreateBranch(tail_block);

    auto type = builder.GetContext().TypeUnion(then_value->GetType(), else_value->GetType());

    builder.SetInsertPoint(then_branch);
    then_value = builder.CreateCast(std::move(then_value), type, true);
    auto then_result = then_value->Load(builder);

    builder.SetInsertPoint(else_branch);
    else_value = builder.CreateCast(std::move(else_value), type, true);
    auto else_result = else_value->Load(builder);

    builder.SetInsertPoint(tail_block);
    const auto phi = builder.CreatePHI(
        type->GenIR(builder),
        {
            { then_block, then_result },
            { else_block, else_result },
    });

    return Value::CreateR(std::move(type), phi);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::TernaryExpression::Reflect(Context& context) const
try
{
    ExpressionPtr condition, value, default_value;

    m_Condition->Reflect(context, condition);
    m_Then->Reflect(context, value);
    m_Else->Reflect(context, default_value);

    return std::make_unique<TernaryExpression>(m_Loc, std::move(condition), std::move(value), std::move(default_value));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream& llove::TernaryExpression::Print(std::ostream& stream) const
{
    return stream << m_Condition << " ? " << m_Then << " : " << m_Else;
}
