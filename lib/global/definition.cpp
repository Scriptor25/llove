#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::DefinitionGlobal::DefinitionGlobal(
    Location loc,
    const bool export_,
    const bool interface,
    const bool implicit,
    std::string name,
    std::vector<Parameter> parameters,
    const bool vararg,
    Field result,
    StatementPtr content)
    : Global(std::move(loc)),
      m_Export(export_),
      m_Interface(interface),
      m_Implicit(implicit),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_VarArg(vararg),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

void llove::DefinitionGlobal::Gen(Builder &builder) const try
{
    builder.GenFunction(
        {
            .Loc = m_Loc,
            .Export = m_Export,
            .Interface = m_Interface,
            .Implicit = m_Implicit,
            .Class = nullptr,
            .Mutable = false,
            .Expose = false,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .VarArg = m_VarArg,
            .Result = m_Result,
            .Content = m_Content.get(),
        }
    );
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::pair<std::string, llove::ValuePtr> llove::DefinitionGlobal::GenImport(
    Context &context,
    Builder &builder,
    const std::string &as,
    const std::map<std::string, std::string> &symbols) const
{
    if (!m_Export)
    {
        Assert(!symbols.contains(m_Name), m_Loc, "symbol is not marked for export");
        return {};
    }

    if (as.empty() && !symbols.empty() && !symbols.contains(m_Name))
        return {};

    const auto register_ = (as.empty() && symbols.empty())
                           || (as.empty() && symbols.contains(m_Name) && symbols.at(m_Name) == m_Name);

    const auto function = builder.GenFunction(
        {
            .Loc = m_Loc,
            .Register = register_,
            .Export = true,
            .Interface = m_Interface,
            .Implicit = m_Implicit,
            .Class = nullptr,
            .Mutable = false,
            .Expose = false,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .VarArg = m_VarArg,
            .Result = m_Result,
            .Content = nullptr,
        }
    );

    if (register_)
        return {};

    auto value = Value::CreateR(function.Type, function.Callee);

    if (symbols.contains(m_Name))
    {
        builder.SetValue(symbols.at(m_Name), std::move(value));
        return {};
    }

    return { m_Name, std::move(value) };
}

std::ostream &llove::DefinitionGlobal::Print(std::ostream &stream) const
{
    stream
            << (m_Export ? "export " : "")
            << (m_Interface ? "interface " : "define ")
            << (m_Implicit ? "implicit " : "")
            << m_Name
            << '(';

    for (auto i = m_Parameters.begin(); i != m_Parameters.end(); ++i)
    {
        if (i != m_Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (m_VarArg)
    {
        if (!m_Parameters.empty())
            stream << ", ";
        stream << "...";
    }

    stream << "): " << m_Result;

    if (!m_Content)
        return stream << ';';
    return stream << ' ' << m_Content;
}
