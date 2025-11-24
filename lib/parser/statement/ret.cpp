#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseRetStatement(const bool inline_)
{
    auto loc = Expect(TokenType_Symbol, "ret").Loc;
    if (!inline_ && SkipIf(TokenType_Other, ";"))
        return std::make_unique<RetStatement>(std::move(loc), nullptr);

    auto value = ParseExpression();

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<RetStatement>(std::move(loc), std::move(value));
}
