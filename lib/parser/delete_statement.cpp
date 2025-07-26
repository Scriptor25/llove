#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseDeleteStatement(const bool inline_)
{
    Expect(TokenType_Symbol, "delete");

    auto value = ParseExpression();

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<DeleteStatement>(std::move(value));
}
