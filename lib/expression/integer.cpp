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
    auto type = m_Type ? As<IntegerType>(m_Type) : nullptr;
    if (!type)
    {
        if (expect && expect->IsInteger())
            type = As<IntegerType>(expect);
        else
            type = builder.GetTypes().GetInteger(false, 64);
    }

    builder.EmitLoc(m_Loc);

    const auto value = llvm::ConstantInt::get(type->Gen(builder), m_Value, type->IsSigned());
    return Value::CreateR(std::move(type), value);
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::IntegerExpression::Reflect(Builder &builder) const try
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<IntegerExpression>(m_Loc, m_Value, std::move(type));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

std::ostream &llove::IntegerExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << m_Value << ':' << m_Type;
    return stream << m_Value;
}
