#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseBreakStatement(const bool is_inline)
{
    auto token = Expect(TokenType_Symbol, "break");

    if (!is_inline)
        Expect(TokenType_Other, ";");

    return std::make_unique<BreakStatement>(std::move(token.Loc));
}
