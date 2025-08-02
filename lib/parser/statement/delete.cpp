#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseDeleteStatement(const bool inline_)
{
    auto loc = Expect(TokenType_Symbol, "delete").Loc;

    auto value = ParseExpression();

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<DeleteStatement>(std::move(loc), std::move(value));
}
