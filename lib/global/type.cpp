#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>

llove::TypeGlobal::TypeGlobal(
    Location loc,
    const bool is_export,
    std::string name,
    TypePtr type)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_Name(std::move(name)),
      m_Type(std::move(type))
{
}

std::string llove::TypeGlobal::GetName() const
{
    return m_Name;
}
llove::GlobalPtr llove::TypeGlobal::Reflect(Context& context) const
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    return std::make_unique<TypeGlobal>(m_Loc, m_IsExport, m_Name, std::move(type));
}

void llove::TypeGlobal::Gen(Builder& builder) const
{
    builder.GetContext().SetNamed(m_Name, m_Type);
}

llove::TemplateInstancePtr llove::TypeGlobal::GenTemplate(
    Builder* /* builder */,
    Context& context,
    const std::string name) const
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    context.SetNamed(name, type);

    return std::make_unique<TypeTemplateInstance>(std::move(type));
}

llove::Import llove::TypeGlobal::GenImport(
    Context& context,
    Builder& /* builder */,
    const std::string& as,
    const ImportSymbols& symbols) const
{
    context.SetNamed(m_Name, m_Type);

    if (!m_IsExport)
        return {};

    if (!as.empty() && !symbols.empty() && !symbols.contains(m_Name))
        return {};

    context.GetParent()->SetNamed(symbols.contains(m_Name) ? symbols.at(m_Name) : m_Name, m_Type);
    return { m_Name, nullptr };
}

std::ostream& llove::TypeGlobal::Print(std::ostream& stream) const
{
    return stream << (m_IsExport ? "export " : "") << "type " << m_Name << " = " << m_Type << ';';
}
