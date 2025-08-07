#include <llove/parser.hpp>

llove::StatementPtr llove::Parser::ParseWhileStatement(const bool inline_)
{
    auto token = Expect(TokenType_Symbol, "while");

    Expect(TokenType_Other, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Other, ")");

    auto content = ParseStatement(inline_);

    return std::make_unique<WhileStatement>(std::move(token.Loc), std::move(condition), std::move(content));
}
