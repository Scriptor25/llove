#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::TypeGlobal::TypeGlobal(Location loc, const bool export_, std::string name, TypePtr type)
    : Global(std::move(loc)),
      m_Export(export_),
      m_Name(std::move(name)),
      m_Type(std::move(type))
{
}

void llove::TypeGlobal::Gen(Builder &builder) const
{
    builder.GetContext().SetNamed(m_Name, m_Type);
}

std::pair<std::string, llove::ValuePtr> llove::TypeGlobal::GenImport(
    Context &context,
    Builder &builder,
    const std::string &as,
    const std::map<std::string, std::string> &symbols) const
{
    context.SetNamed(m_Name, m_Type);

    if (!m_Export)
    {
        Assert(!symbols.contains(m_Name), m_Loc, "symbol is not marked for export");
        return {};
    }

    if ((as.empty() && symbols.empty()) || symbols.contains(m_Name))
        builder.GetContext().SetNamed(symbols.contains(m_Name) ? symbols.at(m_Name) : m_Name, m_Type);

    return {};
}

std::ostream &llove::TypeGlobal::Print(std::ostream &stream) const
{
    return stream << (m_Export ? "export " : "") << "type " << m_Name << " = " << m_Type << ';';
}
