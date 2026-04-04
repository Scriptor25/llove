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

llove::ValuePtr llove::SymbolExpression::GenVal(Builder &builder, TypePtr /* expect */) const try
{
    if (builder.HasValue(m_Name))
        return builder.GetValue(m_Name);

    const auto functions = builder.GetFunctions(m_Name);
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
    std::vector<FunctionReference> candidates;

    TypePtr value_type;
    ValuePtr self;

    if (builder.HasValue(m_Name))
    {
        auto value = builder.GetValue(m_Name);
        value_type = value->GetType();

        if (value_type->IsFunction())
            candidates.push_back(
                {
                    .IsPublic = false,
                    .IsImplicit = false,
                    .Name = m_Name,
                    .Type = As<FunctionType>(value_type),
                    .Callee = value->Load(builder),
                });
        else if (value_type->IsClass())
        {
            auto functions = builder.GetFunctions("()", value->AsField());
            candidates.insert(candidates.end(), functions.begin(), functions.end());
            self = std::move(value);
        }
    }

    auto functions = builder.GetFunctions(m_Name);
    candidates.insert(candidates.end(), functions.begin(), functions.end());

    Assert(
        !candidates.empty() || !value_type,
        "illegal callee symbol '{}', type '{}' is not a function type",
        m_Name,
        value_type);
    Assert(!candidates.empty(), "undefined symbol '{}'", m_Name);
    return { std::move(candidates), std::move(self) };
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::SymbolExpression::Reflect(Context & /* context */) const try
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
