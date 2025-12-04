#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseClassType()
{
    Expect(TokenType_Symbol, "class");

    auto name = Expect(TokenType_Symbol).Value;
    return m_Context.GetClass(std::move(name));
}
