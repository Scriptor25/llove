#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::DefinitionGlobal::DefinitionGlobal(
    Location loc,
    const bool interface,
    const bool implicit,
    std::string name,
    std::vector<Parameter> parameters,
    const bool vararg,
    Field result,
    StatementPtr content)
    : Global(std::move(loc)),
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
            .Interface = m_Interface,
            .Implicit = m_Implicit,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .VarArg = m_VarArg,
            .Result = m_Result,
            .Content = m_Content.get(),
        }
    );
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

std::ostream &llove::DefinitionGlobal::Print(std::ostream &stream) const
{
    stream
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
    stream << ')';
    if (m_Result)
        stream << ": " << m_Result;
    if (!m_Content)
        return stream << ';';
    return stream << ' ' << m_Content;
}
