#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::FloatExpression::FloatExpression(
    Location loc,
    const double_t value,
    TypePtr type)
    : Expression(std::move(loc)),
      m_Value(value),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::FloatExpression::GenVal(
    Builder &builder,
    TypePtr expect) const try
{
    auto type = m_Type
                    ? As<FloatType>(m_Type)
                    : expect && expect->IsFloat()
                    ? As<FloatType>(std::move(expect))
                    : builder.GetContext().GetFloat(64);

    builder.EmitLoc(m_Loc);

    const auto value = llvm::ConstantFP::get(type->GenIR(builder), m_Value);
    return Value::CreateR(std::move(type), value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::FloatExpression::Reflect(Context &context) const try
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    return std::make_unique<FloatExpression>(m_Loc, m_Value, std::move(type));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::FloatExpression::Print(std::ostream &stream) const
{
    stream << m_Value;
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}
