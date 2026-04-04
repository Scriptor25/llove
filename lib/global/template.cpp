#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>

llove::TemplateGlobal::TemplateGlobal(
    Location loc,
    bool is_export,
    std::vector<TemplateParameter> parameters,
    GlobalPtr content)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_Parameters(std::move(parameters)),
      m_Content(std::move(content))
{
}

std::string llove::TemplateGlobal::GetName() const
{
    return {};
}

llove::GlobalPtr llove::TemplateGlobal::Reflect(Context &context) const
{
    GlobalPtr content;
    m_Content->Reflect(context, content);

    return std::make_unique<TemplateGlobal>(m_Loc, m_IsExport, m_Parameters, std::move(content));
}

void llove::TemplateGlobal::Gen(Builder &builder) const
{
    auto &context = builder.GetContext();

    auto name = m_Content->GetName();

    GlobalPtr content;
    m_Content->Reflect(context, content);

    context.CreateTemplate(name, m_Parameters, std::move(content));
}

llove::TemplateInstancePtr llove::TemplateGlobal::GenTemplate(
    Builder * /* builder */,
    Context & /* context */,
    std::string /* name */) const
{
    Error(m_Loc, "templates do not support templating");
}

llove::Import llove::TemplateGlobal::GenImport(
    Context &context,
    Builder &builder,
    const std::string &as,
    const ImportSymbols &symbols) const
{
    auto name = m_Content->GetName();

    GlobalPtr content;
    m_Content->Reflect(context, content);

    context.CreateTemplate(name, m_Parameters, std::move(content));

    if (!m_IsExport)
        return {};

    if (as.empty() && !symbols.empty() && !symbols.contains(name))
        return {};

    if (symbols.contains(name))
        name = symbols.at(name);

    context.GetParent()->CreateTemplate(
        name,
        m_Parameters,
        m_Content->Reflect(*context.GetParent()));

    return { name, nullptr };
}

std::ostream &llove::TemplateGlobal::Print(std::ostream &stream) const
{
    if (m_IsExport)
        stream << "export ";

    stream << "template<";
    for (auto it = m_Parameters.begin(); it != m_Parameters.end(); ++it)
    {
        if (it != m_Parameters.begin())
            stream << ", ";
        stream << it->first;
    }
    return stream << '>' << std::endl << m_Content;
}
