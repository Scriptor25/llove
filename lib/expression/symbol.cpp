#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::SymbolExpression::SymbolExpression(std::string name)
    : m_Name(std::move(name))
{
}

llove::ValuePtr llove::SymbolExpression::GenVal(Builder &builder, TypePtr expect) const
{
    // TODO: if no symbol with name exists, return single function with name if exists

    auto value = builder.GetValue(m_Name);
    Assert(value != nullptr, "undefined symbol name '{}'", m_Name);
    return value;
}

llove::CalleeInfo llove::SymbolExpression::GenCallee(Builder &builder) const
{
    // TODO: if symbol with name exists, add to candidates

    auto candidates = builder.GetFunctions(m_Name);
    Assert(!candidates.empty(), "undefined symbol name '{}'", m_Name);
    return { .Candidates = std::move(candidates) };
}

std::ostream &llove::SymbolExpression::Print(std::ostream &stream) const
{
    return stream << m_Name;
}
