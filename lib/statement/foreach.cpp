#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::ForEachStatement::ForEachStatement(
    const bool mutable_,
    const bool reference,
    std::string name,
    ExpressionPtr range,
    StatementPtr content)
    : m_Mutable(mutable_),
      m_Reference(reference),
      m_Name(std::move(name)),
      m_Range(std::move(range)),
      m_Content(std::move(content))
{
}

void llove::ForEachStatement::Gen(Builder &builder) const
{
    Error("not yet implemented");
}

std::ostream &llove::ForEachStatement::Print(std::ostream &stream) const
{
    return stream
           << "foreach ("
           << (m_Mutable ? "mut " : "")
           << (m_Reference ? "&" : "")
           << m_Name
           << " : "
           << m_Range
           << ") "
           << m_Content;
}
