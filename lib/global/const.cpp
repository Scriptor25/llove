#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ConstGlobal::ConstGlobal(Location loc, std::string name, TypePtr type, ExpressionPtr value)
    : Global(std::move(loc)),
      m_Name(std::move(name)),
      m_Type(std::move(type)),
      m_Value(std::move(value))
{
}

void llove::ConstGlobal::Gen(Builder &builder) const
{
    auto value = m_Value->GenVal(builder, m_Type);
    auto type = m_Type ? m_Type : value->GetType();

    value = builder.CreateCast(std::move(value), std::move(type), true);

    // TODO: generate debug info
    builder.SetValue(m_Name, std::move(value));
}

std::ostream &llove::ConstGlobal::Print(std::ostream &stream) const
{
    stream << "const " << m_Name;

    if (m_Type)
        stream << ": " << m_Type;

    return stream << " = " << m_Value << ';';
}
