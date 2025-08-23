#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SubscriptExpression::SubscriptExpression(Location loc, ExpressionPtr value, ExpressionPtr index)
    : Expression(std::move(loc)),
      m_Value(std::move(value)),
      m_Index(std::move(index))
{
}

llove::ValuePtr llove::SubscriptExpression::GenVal(Builder &builder, TypePtr) const try
{
    const auto value = m_Value->GenVal(builder, nullptr);
    const auto index = m_Index->GenVal(builder, nullptr);

    builder.EmitLoc(m_Loc);

    switch (auto type = value->GetType(); type->GetId())
    {
    case TypeId_Pointer:
        return builder.GetPointerElement(value, index);
    case TypeId_Array:
        return builder.GetArrayElement(value, index);
    default:
        Error("subscript on non-pointer and non-array value of type {}", type);
    }
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::SubscriptExpression::Reflect(Context &context) const try
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(context, value);

    ExpressionPtr index;
    if (m_Index)
        m_Index->Reflect(context, index);

    return std::make_unique<SubscriptExpression>(m_Loc, std::move(value), std::move(index));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::SubscriptExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '[' << m_Index << ']';
}
