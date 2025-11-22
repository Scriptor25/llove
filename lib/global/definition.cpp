#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <utility>

llove::DefinitionGlobal::DefinitionGlobal(
    Location loc,
    const bool is_export,
    const bool is_interface,
    const bool is_implicit,
    std::string name,
    std::vector<Parameter> parameters,
    std::pair<
        bool,
        std::string> variadic,
    Field result,
    StatementPtr content)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_IsInterface(is_interface),
      m_IsImplicit(is_implicit),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_Variadic(std::move(variadic)),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

void llove::DefinitionGlobal::Gen(Builder& builder) const
try
{
    StatementPtr content;
    if (m_Content)
        m_Content->Reflect(builder.GetContext(), content);

    builder.GenFunction(
        {
            .Loc = m_Loc,
            .IsExport = m_IsExport,
            .IsInterface = m_IsInterface,
            .IsImplicit = m_IsImplicit,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .Variadic = m_Variadic,
            .Result = m_Result,
            .Content = std::move(content),
        },
        true);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::pair<
    std::string,
    llove::ValuePtr>
llove::DefinitionGlobal::GenImport(
    Context& context,
    Builder& builder,
    const std::string& as,
    const std::map<
        std::string,
        std::string>& symbols) const
{
    if (!m_IsExport)
        return {};

    if (as.empty() && !symbols.empty() && !symbols.contains(m_Name))
        return {};

    const auto register_function = (as.empty() && symbols.empty())
                                || (as.empty() && symbols.contains(m_Name) && symbols.at(m_Name) == m_Name);

    const auto function = builder.GenFunction(
        {
            .Loc = m_Loc,
            .IsExport = true,
            .IsInterface = m_IsInterface,
            .IsImplicit = m_IsImplicit,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .Variadic = m_Variadic,
            .Result = m_Result,
        },
        register_function);

    if (register_function)
        return {};

    auto value = Value::CreateR(function.Type, function.Callee);

    if (symbols.contains(m_Name))
    {
        builder.SetValue(symbols.at(m_Name), std::move(value));
        return {};
    }

    return { m_Name, std::move(value) };
}

std::ostream& llove::DefinitionGlobal::Print(std::ostream& stream) const
{
    stream << (m_IsExport ? "export " : "") << (m_IsInterface ? "interface " : "define ") << (m_IsImplicit ? "implicit " : "") << m_Name << '(';

    for (auto i = m_Parameters.begin(); i != m_Parameters.end(); ++i)
    {
        if (i != m_Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (m_Variadic.first)
    {
        if (!m_Parameters.empty())
            stream << ", ";
        stream << "...";
        if (!m_Variadic.second.empty())
            stream << m_Variadic.second;
    }

    stream << "): " << m_Result;

    if (!m_Content)
        return stream << ';';
    return stream << ' ' << m_Content;
}
