#include <llove/context.hpp>
#include <llove/parser.hpp>

void llove::Parser::ParseTypeAlias()
{
    Expect(TokenType_Symbol, "type");

    const auto name = Expect(TokenType_Symbol).Value;
    Expect(TokenType_Operator, "=");
    m_Context.Set(name, ParseType());
}
