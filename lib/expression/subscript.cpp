#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SubscriptExpression::SubscriptExpression(ExpressionPtr value, ExpressionPtr index)
    : m_Value(std::move(value)),
      m_Index(std::move(index))
{
}

llove::ValuePtr llove::SubscriptExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    const auto value = m_Value->GenVal(builder, expect ? builder.GetTypes().GetPointer(expect, false) : nullptr);
    const auto index = m_Index->GenVal(builder, nullptr);

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

std::ostream &llove::SubscriptExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '[' << m_Index << ']';
}
