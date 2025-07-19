#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::ArrayExpression::ArrayExpression(std::vector<ExpressionPtr> values, ArrayType::Ptr type)
    : m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::ArrayExpression::GenVal(Builder &builder, TypePtr expect) const
{
    Error("not yet implemented");
}

std::ostream &llove::ArrayExpression::Print(std::ostream &stream) const
{
    stream << "[ ";
    for (auto i = m_Values.begin(); i != m_Values.end(); ++i)
    {
        if (i != m_Values.begin())
            stream << ", ";
        stream << *i;
    }
    stream << " ]";
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}
