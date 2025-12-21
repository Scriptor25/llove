#include <llove/builder.hpp>
#include <llove/forward.hpp>
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

std::string llove::ConstGlobal::GetName() const
{
    return m_Name;
}

llove::GlobalPtr llove::ConstGlobal::Reflect(Context& context) const
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    ExpressionPtr value;
    m_Value->Reflect(context, value);

    return std::make_unique<ConstGlobal>(m_Loc, m_IsExport, m_Name, std::move(type), std::move(value));
}

void llove::ConstGlobal::Gen(Builder& builder) const
{
    auto value = m_Value->GenVal(builder, m_Type);
    auto type = m_Type ? m_Type : value->GetType();

    value = builder.CreateCast(std::move(value), std::move(type), true);

    // TODO: generate debug info
    builder.SetValue(m_Name, std::move(value));
}

llove::TemplateInstancePtr llove::ConstGlobal::GenTemplate(
    Builder* /* builder */,
    Context& /* context */,
    std::string /* name */) const
{
    Error(m_Loc, "consts do not support templating");
}

llove::Import llove::ConstGlobal::GenImport(
    Context& /* context */,
    Builder& builder,
    const std::string& as,
    const ImportSymbols& symbols) const
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
