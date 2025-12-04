#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseRetStatement(const bool is_inline)
{
    auto loc = Expect(TokenType_Symbol, "ret").Loc;
    if (!is_inline && SkipIf(TokenType_Other, ";"))
        return std::make_unique<RetStatement>(std::move(loc), nullptr);

    auto value = ParseExpression();

    if (!is_inline)
        Expect(TokenType_Other, ";");

    return std::make_unique<RetStatement>(std::move(loc), std::move(value));
}
