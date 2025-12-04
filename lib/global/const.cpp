#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ConstGlobal::ConstGlobal(
    Location loc,
    const bool is_export,
    std::string name,
    TypePtr type,
    ExpressionPtr value)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_Name(std::move(name)),
      m_Type(std::move(type)),
      m_Value(std::move(value))
{
}

void llove::ConstGlobal::Gen(Builder& builder) const
{
    auto value = m_Value->GenVal(builder, m_Type);
    auto type = m_Type ? m_Type : value->GetType();

    value = builder.CreateCast(std::move(value), std::move(type), true);

    // TODO: generate debug info
    builder.SetValue(m_Name, std::move(value));
}

std::pair<
    std::string,
    llove::ValuePtr>
llove::ConstGlobal::GenImport(
    Context& /* context */,
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

    auto value = m_Value->GenVal(builder, m_Type);
    auto type = m_Type ? m_Type : value->GetType();

    value = builder.CreateCast(std::move(value), std::move(type), true);

    if ((as.empty() && symbols.empty()) || (symbols.contains(m_Name) && symbols.at(m_Name) == m_Name))
    {
        builder.SetValue(symbols.contains(m_Name) ? symbols.at(m_Name) : m_Name, std::move(value));
        return { m_Name, nullptr };
    }

    return { m_Name, std::move(value) };
}

std::ostream& llove::ConstGlobal::Print(std::ostream& stream) const
{
    stream << (m_IsExport ? "export " : "") << "const " << m_Name;

    if (m_Type)
        stream << ": " << m_Type;

    return stream << " = " << m_Value << ';';
}
