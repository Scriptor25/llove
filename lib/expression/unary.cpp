#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::UnaryExpression::UnaryExpression(std::string operator_, ExpressionPtr operand, const bool suffix)
    : m_Operator(std::move(operator_)),
      m_Operand(std::move(operand)),
      m_Suffix(suffix)
{
}

llove::ValuePtr llove::UnaryExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    auto operand = m_Operand->GenVal(builder, expect);

    if (m_Operator == "$")
    {
        Assert(operand->IsReferenceable(), "cannot remove ownership from rvalue");

        builder.PopDestructor(operand->GetPointer());
        const auto value = operand->Load(builder);
        return Value::CreateR(operand->GetType(), value);
    }

    if (const auto operator_ = builder.FindOperator(m_Operator, operand->AsField(), m_Suffix))
        return (*operator_)(builder, std::move(operand));

    Error(
        "undefined unary operator {}{}{}",
        m_Suffix ? std::string{} : m_Operator,
        operand->GetType(),
        m_Suffix ? m_Operator : std::string{});
}

std::ostream &llove::UnaryExpression::Print(std::ostream &stream) const
{
    if (m_Suffix)
        return stream << m_Operand << m_Operator;
    return stream << m_Operator << m_Operand;
}
