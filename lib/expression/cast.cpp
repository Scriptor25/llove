#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::CastExpression::CastExpression(
    Location loc,
    ExpressionPtr value,
    TypePtr type)
    : Expression(std::move(loc)),
      m_Value(std::move(value)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::CastExpression::GenVal(
    Builder& builder,
    TypePtr /* expect */) const
try
{
    auto value = m_Value->GenVal(builder, m_Type);
    return builder.CreateCast(std::move(value), m_Type, false);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::CastExpression::Reflect(Context& context) const
try
{
    ExpressionPtr value;
    TypePtr type;

    m_Value->Reflect(context, value);
    Type::Reflect(context, m_Type, type);

    return std::make_unique<CastExpression>(m_Loc, std::move(value), std::move(type));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream& llove::CastExpression::Print(std::ostream& stream) const
{
    return stream << m_Value << " as " << m_Type;
}
