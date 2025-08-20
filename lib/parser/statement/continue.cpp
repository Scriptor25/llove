#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseContinueStatement(const bool inline_)
{
    auto token = Expect(TokenType_Symbol, "continue");

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<ContinueStatement>(std::move(token.Loc));
}
