#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::UnaryExpression::UnaryExpression(Location loc, std::string operator_, ExpressionPtr operand, const bool suffix)
    : Expression(std::move(loc)),
      m_Operator(std::move(operator_)),
      m_Operand(std::move(operand)),
      m_Suffix(suffix)
{
}

llove::ValuePtr llove::UnaryExpression::GenVal(Builder &builder, const TypePtr expect) const try
{
    auto operand = m_Operand->GenVal(builder, expect);

    builder.EmitLoc(m_Loc);

    if (m_Operator == "$")
    {
        if (!operand->IsReferenceable())
            return operand;
        if (!operand->GetType()->IsClass())
            return Value::CreateR(operand->GetType(), operand->Load(builder));
        Assert(operand->IsMutable(), "cannot remove ownership from immutable lvalue");
    }

    if (const auto operator_ = builder.FindOperator(m_Operator, operand->AsField(), m_Suffix))
        return (*operator_)(builder, std::move(operand));

    Error(
        "undefined unary operator {}{}{}",
        m_Suffix ? std::string{} : m_Operator,
        operand->AsField(),
        m_Suffix ? m_Operator : std::string{});
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::UnaryExpression::Reflect(Builder &builder) const try
{
    ExpressionPtr operand;
    if (m_Operand)
        m_Operand->Reflect(builder, operand);

    return std::make_unique<UnaryExpression>(m_Loc, m_Operator, std::move(operand), m_Suffix);
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

std::ostream &llove::UnaryExpression::Print(std::ostream &stream) const
{
    if (m_Suffix)
        return stream << m_Operand << m_Operator;
    return stream << m_Operator << m_Operand;
}
