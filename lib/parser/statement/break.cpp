#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseBreakStatement(const bool inline_)
{
    auto token = Expect(TokenType_Symbol, "break");

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<BreakStatement>(std::move(token.Loc));
}
