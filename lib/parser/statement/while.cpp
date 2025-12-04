#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseWhileStatement(const bool is_inline)
{
    auto token = Expect(TokenType_Symbol, "while");

    Expect(TokenType_Other, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Other, ")");

    auto content = ParseStatement(is_inline);

    return std::make_unique<WhileStatement>(std::move(token.Loc), std::move(condition), std::move(content));
}
