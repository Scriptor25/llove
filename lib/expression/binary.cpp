#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::BinaryExpression::BinaryExpression(std::string operator_, ExpressionPtr left, ExpressionPtr right)
    : m_Operator(std::move(operator_)),
      m_Left(std::move(left)),
      m_Right(std::move(right))
{
}

llove::ValuePtr llove::BinaryExpression::GenVal(Builder &builder, TypePtr expect) const
{
    static const std::map<std::string_view, const char *> assign
    {
        { "+=", "+" },
        { "-=", "-" },
        { "*=", "*" },
        { "/=", "/" },
        { "%=", "%" },
        { "&=", "&" },
        { "|=", "|" },
        { "^=", "^" },
        { "<<=", "<<" },
        { ">>=", ">>" },
    };

    auto left = m_Left->GenVal(builder, nullptr);
    auto right = m_Right->GenVal(builder, left->GetType());

    if (const auto operator_ = builder.FindOperator(m_Operator, left->AsField(), right->AsField()))
        return (*operator_)(builder, std::move(left), std::move(right));

    if (assign.contains(m_Operator))
        if (const auto operator_ = builder.FindOperator(assign.at(m_Operator), left->AsField(), right->AsField()))
        {
            const auto value = (*operator_)(builder, left, std::move(right));
            left->Store(builder, value);
            return left;
        }

    Error("undefined binary operator {} {} {}", left->GetType(), m_Operator, right->GetType());
}

std::ostream &llove::BinaryExpression::Print(std::ostream &stream) const
{
    return stream << m_Left << ' ' << m_Operator << ' ' << m_Right;
}
