#include <llove/builder.hpp>
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

    const auto functions = builder.FindFunctions(m_Name);
    if (functions.empty())
        Error("undefined symbol '{}'", m_Name);
    if (functions.size() > 1)
        Error("ambiguous function symbol '{}'", m_Name);

    builder.EmitLoc(m_Loc);

    const auto &function = functions.front();
    return Value::CreateR(function.Type, function.Callee);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::CalleeInfo llove::SymbolExpression::GenCallee(Builder &builder) const try
{
    // TODO: if symbol with name exists, add to candidates

    std::vector<FunctionReference> candidates;

    TypePtr symbol_type;
    if (builder.HasValue(m_Name))
    {
        const auto value = builder.GetValue(m_Name);
        symbol_type = value->GetType();

        if (symbol_type->IsFunction())
        {
            FunctionReference reference
            {
                .IsExposed = false,
                .IsImplicit = false,
                .Name = m_Name,
                .Type = As<FunctionType>(symbol_type),
                .Callee = value->Load(builder),
            };

            candidates.emplace_back(std::move(reference));
        }
    }

    auto functions = builder.FindFunctions(m_Name);
    candidates.insert(candidates.end(), functions.begin(), functions.end());

    Assert(!candidates.empty() || !symbol_type, "illegal callee symbol '{}', type '{}' is not a function type", m_Name, symbol_type);
    Assert(!candidates.empty(), "undefined symbol '{}'", m_Name);
    return { .Candidates = std::move(candidates) };
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::SymbolExpression::Reflect(Context &context) const try
{
    return std::make_unique<SymbolExpression>(m_Loc, m_Name);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::SymbolExpression::Print(std::ostream &stream) const
{
    return stream << m_Name;
}
