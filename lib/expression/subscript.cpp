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

llove::ValuePtr llove::SubscriptExpression::GenVal(Builder &builder, const TypePtr expect) const try
{
    const auto value = m_Value->GenVal(builder, expect ? builder.GetTypes().GetPointer(expect, false) : nullptr);
    const auto index = m_Index->GenVal(builder, nullptr);

    builder.EmitLoc(m_Loc);

    switch (value->GetType()->GetId())
    {
    case TypeId_Pointer:
        return builder.CreatePointerElement(value, index);
    case TypeId_Array:
        return builder.CreateArrayElement(value, index);
    default:
        Error("subscript on non-pointer and non-array value of type {}", value->GetType());
    }
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::SubscriptExpression::Reflect(Builder &builder) const try
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(builder, value);

    ExpressionPtr index;
    if (m_Index)
        m_Index->Reflect(builder, index);

    return std::make_unique<SubscriptExpression>(m_Loc, std::move(value), std::move(index));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

std::ostream &llove::SubscriptExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '[' << m_Index << ']';
}
