#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Constants.h>

llove::IntExpression::IntExpression(Location loc, const uint64_t value, TypePtr type)
    : Expression(std::move(loc)),
      m_Value(value),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::IntExpression::GenVal(Builder &builder, const TypePtr expect) const
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

llove::StatementPtr llove::IntExpression::Reflect(Builder &builder) const
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<IntExpression>(m_Loc, m_Value, std::move(type));
}

std::ostream &llove::IntExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << m_Value << ':' << m_Type;
    return stream << m_Value;
}
