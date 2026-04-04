#include <algorithm>

#include <llove/error.hpp>
#include <llove/parser.hpp>

llove::Token &llove::Parser::Pop()
{
    return m_Token = Next();
}

llove::Token llove::Parser::Skip()
{
    auto token = m_Token;
    Pop();
    return token;
}

bool llove::Parser::At(const TokenType type, const std::string &value) const
{
    return m_Token.Type == type && (value.empty() || m_Token.Value == value);
}

bool llove::Parser::At(const TokenType type, const std::vector<std::string> &values) const
{
    if (m_Token.Type != type)
        return false;
    return std::ranges::any_of(
        values,
        [&](const std::string &value)
        {
            return m_Token.Value == value;
        });
}

bool llove::Parser::SkipIf(const TokenType type, const std::string &value)
{
    if (At(type, value))
    {
        Pop();
        return true;
    }
    return false;
}

llove::Token llove::Parser::Expect(TokenType type, const std::string &value)
{
    Assert(
        At(type, value),
        m_Token.Loc,
        "expected {} : '{}', but is {} : '{}'",
        type,
        value,
        m_Token.Type,
        m_Token.Value);
    return Skip();
}

llove::Token llove::Parser::Expect(TokenType type, const std::vector<std::string> &values)
{
    Assert(
        At(type, values),
        m_Token.Loc,
        "expected {} : [{}], but is {} : '{}'",
        type,
        values,
        m_Token.Type,
        m_Token.Value);
    return Skip();
}
