#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::FloatExpression::FloatExpression(Location loc, const double_t value, TypePtr type)
    : Expression(std::move(loc)),
      m_Value(value),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::FloatExpression::GenVal(Builder &builder, const TypePtr expect) const try
{
    auto type = m_Type ? As<FloatType>(m_Type) : nullptr;
    if (!type)
    {
        if (expect && expect->IsInteger())
            type = As<FloatType>(expect);
        else
            type = builder.GetTypes().GetFloat(64);
    }

    builder.EmitLoc(m_Loc);

    const auto value = llvm::ConstantFP::get(type->Gen(builder), m_Value);
    return Value::CreateR(std::move(type), value);
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::FloatExpression::Reflect(Builder &builder) const try
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<FloatExpression>(m_Loc, m_Value, std::move(type));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

std::ostream &llove::FloatExpression::Print(std::ostream &stream) const
{
    stream << m_Value;
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}
