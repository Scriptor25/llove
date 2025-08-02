#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseYieldStatement(const bool inline_)
{
    auto loc = Expect(TokenType_Symbol, "yield").Loc;
    if (!inline_ && SkipIf(TokenType_Other, ";"))
        return std::make_unique<YieldStatement>(std::move(loc), nullptr);

    auto value = ParseExpression();

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<YieldStatement>(std::move(loc), std::move(value));
}
