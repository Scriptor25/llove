#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::StringExpression::StringExpression(Location loc, std::string value)
    : Expression(std::move(loc)),
      m_Value(std::move(value))
{
}

llove::ValuePtr llove::StringExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    static std::map<std::string, llvm::Value *> string_cache;

    builder.EmitLoc(m_Loc);

    auto &value = string_cache[m_Value];
    if (!value)
        value = builder.GetStr(m_Value);

    return Value::CreateR(builder.GetContext().GetPointer(builder.GetContext().GetInteger(true, 8), false), value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::StringExpression::Reflect(Context &context) const try
{
    return std::make_unique<StringExpression>(m_Loc, m_Value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
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
