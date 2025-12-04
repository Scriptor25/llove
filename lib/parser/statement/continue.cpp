#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseContinueStatement(const bool is_inline)
{
    auto token = Expect(TokenType_Symbol, "continue");

    if (!is_inline)
        Expect(TokenType_Other, ";");

    return std::make_unique<ContinueStatement>(std::move(token.Loc));
}
