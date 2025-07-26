#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::ClassDefinitionGlobal::ClassDefinitionGlobal(
    ClassType::Ptr class_type,
    const bool implicit,
    const bool mutable_,
    std::string name,
    std::vector<Parameter> parameters,
    const bool vararg,
    Field result,
    StatementPtr content)
    : m_ClassType(std::move(class_type)),
      m_Implicit(implicit),
      m_Mutable(mutable_),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_VarArg(vararg),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

void llove::ClassDefinitionGlobal::Gen(Builder &builder) const
{
    std::vector<Field> parameters;
    for (auto &[info_, name_] : m_Parameters)
        parameters.emplace_back(info_);
    const auto class_function = m_ClassType->GetFunction(m_Name, m_Mutable, parameters, m_VarArg, m_Result);

    Assert(class_function.has_value(), "class function prototype mismatch");

    builder.GenFunction(
        {
            .Class = m_ClassType,
            .Mutable = m_Mutable,
            .Expose = class_function->Expose,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .VarArg = m_VarArg,
            .Result = m_Result,
            .Content = m_Content.get(),
        }
    );
}

std::ostream &llove::ClassDefinitionGlobal::Print(std::ostream &stream) const
{
    stream
            << "define:"
            << m_ClassType->GetName()
            << ' '
            << (m_Implicit ? "implicit " : "")
            << (m_Mutable ? "mut " : "")
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
