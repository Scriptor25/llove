#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::StringExpression::StringExpression(Location loc, std::string value)
    : Expression(std::move(loc)),
      m_Value(std::move(value))
{
}

llove::ValuePtr llove::StringExpression::GenVal(Builder &builder, TypePtr expect) const
{
    builder.EmitLoc(m_Loc);

    const auto value = builder.CreateGlobalString(m_Value);
    return Value::CreateR(builder.GetTypes().GetPointer(builder.GetTypes().GetInteger(true, 8), false), value);
}

llove::StatementPtr llove::StringExpression::Reflect(Builder &builder) const
{
    return std::make_unique<StringExpression>(m_Loc, m_Value);
}

std::ostream &llove::StringExpression::Print(std::ostream &stream) const
{
    std::string value;
    for (auto &c : m_Value)
    {
        if (c >= 0x20)
        {
            value += c;
            continue;
        }

        value += '\\';
        value += std::to_string(c / 0100);
        value += std::to_string(c % 0100 / 010);
        value += std::to_string(c % 010);
    }

    return stream << '"' << value << '"';
}
