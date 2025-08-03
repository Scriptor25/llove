#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::BinaryExpression::BinaryExpression(Location loc, std::string operator_, ExpressionPtr left, ExpressionPtr right)
    : Expression(std::move(loc)),
      m_Operator(std::move(operator_)),
      m_Left(std::move(left)),
      m_Right(std::move(right))
{
}

llove::ValuePtr llove::BinaryExpression::GenVal(Builder &builder, TypePtr expect) const try
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

    builder.EmitLoc(m_Loc);

    if (const auto operator_ = builder.FindOperator(m_Operator, left->AsField(), right->AsField()))
        return (*operator_)(builder, std::move(left), std::move(right));

    if (assign.contains(m_Operator))
        if (const auto operator_ = builder.FindOperator(assign.at(m_Operator), left->AsField(), right->AsField()))
        {
            const auto value = (*operator_)(builder, left, std::move(right));
            left->Store(builder, value);
            return left;
        }

    Error("operator '{} {} {}' not implemented", left->AsField(), m_Operator, right->AsField());
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::BinaryExpression::Reflect(Builder &builder) const try
{
    ExpressionPtr left;
    if (m_Left)
        m_Left->Reflect(builder, left);

    ExpressionPtr right;
    if (m_Right)
        m_Right->Reflect(builder, right);

    return std::make_unique<BinaryExpression>(m_Loc, m_Operator, std::move(left), std::move(right));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::BinaryExpression::Print(std::ostream &stream) const
{
    return stream << m_Left << ' ' << m_Operator << ' ' << m_Right;
}
