#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::RangeExpression::RangeExpression(ExpressionPtr begin, ExpressionPtr end)
    : m_Begin(std::move(begin)),
      m_End(std::move(end))
{
}

llove::ValuePtr llove::RangeExpression::GenVal(Builder &builder, TypePtr expect) const
{
    Error("not yet implemented");
}

std::ostream &llove::RangeExpression::Print(std::ostream &stream) const
{
    return stream << m_Begin << ".." << m_End;
}
