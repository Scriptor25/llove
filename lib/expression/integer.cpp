#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Constants.h>

llove::IntegerExpression::IntegerExpression(Location loc, const uint64_t value, TypePtr type)
    : Expression(std::move(loc)),
      m_Value(value),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::IntegerExpression::GenVal(Builder &builder, const TypePtr expect) const try
{
    auto type = m_Type
                    ? As<IntegerType>(m_Type)
                    : expect && expect->IsInteger()
                    ? As<IntegerType>(expect)
                    : builder.GetContext().GetInteger(false, 64);

    builder.EmitLoc(m_Loc);

    const auto value = llvm::ConstantInt::get(type->GenIR(builder), m_Value, type->IsSigned());
    return Value::CreateR(std::move(type), value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::IntegerExpression::Reflect(Context &context) const try
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    return std::make_unique<IntegerExpression>(m_Loc, m_Value, std::move(type));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::IntegerExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << m_Value << ':' << m_Type;
    return stream << m_Value;
}
