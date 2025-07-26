#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SymbolExpression::SymbolExpression(std::string name)
    : m_Name(std::move(name))
{
}

llove::ValuePtr llove::SymbolExpression::GenVal(Builder &builder, TypePtr expect) const
{
    if (builder.HasValue(m_Name))
        return builder.GetValue(m_Name);

    const auto functions = builder.GetFunctions(m_Name);
    if (functions.empty())
        Error("undefined symbol name '{}'", m_Name);
    if (functions.size() > 1)
        Error("ambiguous function symbol name '{}'", m_Name);

    const auto &function = functions.front();
    return Value::CreateR(function.Type, function.Callee);
}

llove::CalleeInfo llove::SymbolExpression::GenCallee(Builder &builder) const
{
    // TODO: if symbol with name exists, add to candidates

    auto candidates = builder.GetFunctions(m_Name);
    Assert(!candidates.empty(), "undefined symbol '{}'", m_Name);
    return { .Candidates = std::move(candidates) };
}

llove::StatementPtr llove::SymbolExpression::Reflect(Context &types) const
{
    return std::make_unique<SymbolExpression>(m_Name);
}

std::ostream &llove::SymbolExpression::Print(std::ostream &stream) const
{
    return stream << m_Name;
}
