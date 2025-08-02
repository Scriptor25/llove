#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SymbolExpression::SymbolExpression(Location loc, std::string name)
    : Expression(std::move(loc)),
      m_Name(std::move(name))
{
}

llove::ValuePtr llove::SymbolExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    if (builder.HasValue(m_Name))
        return builder.GetValue(m_Name);

    const auto functions = builder.GetFunctions(m_Name);
    if (functions.empty())
        Error("undefined symbol name '{}'", m_Name);
    if (functions.size() > 1)
        Error("ambiguous function symbol name '{}'", m_Name);

    builder.EmitLoc(m_Loc);

    const auto &function = functions.front();
    return Value::CreateR(function.Type, function.Callee);
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::CalleeInfo llove::SymbolExpression::GenCallee(Builder &builder) const try
{
    // TODO: if symbol with name exists, add to candidates

    auto candidates = builder.GetFunctions(m_Name);
    Assert(!candidates.empty(), "undefined symbol '{}'", m_Name);
    return { .Candidates = std::move(candidates) };
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::SymbolExpression::Reflect(Builder &builder) const try
{
    return std::make_unique<SymbolExpression>(m_Loc, m_Name);
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

std::ostream &llove::SymbolExpression::Print(std::ostream &stream) const
{
    return stream << m_Name;
}
