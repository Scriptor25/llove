#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseNamedType()
{
    const auto token = Expect(TokenType_Symbol);

    const auto &name = token.Value;
    if (auto type = m_Context.GetNamed(name))
        return type;

    if (name == "void")
        return m_Context.GetVoid();
    if (name == "i1")
        return m_Context.GetInteger(true, 1);
    if (name == "i8")
        return m_Context.GetInteger(true, 8);
    if (name == "i16")
        return m_Context.GetInteger(true, 16);
    if (name == "i32")
        return m_Context.GetInteger(true, 32);
    if (name == "i64")
        return m_Context.GetInteger(true, 64);
    if (name == "u1")
        return m_Context.GetInteger(false, 1);
    if (name == "u8")
        return m_Context.GetInteger(false, 8);
    if (name == "u16")
        return m_Context.GetInteger(false, 16);
    if (name == "u32")
        return m_Context.GetInteger(false, 32);
    if (name == "u64")
        return m_Context.GetInteger(false, 64);
    if (name == "f16")
        return m_Context.GetFloat(16);
    if (name == "f32")
        return m_Context.GetFloat(32);
    if (name == "f64")
        return m_Context.GetFloat(64);

    Error(token.Loc, "undefined type '{}'", name);
}
