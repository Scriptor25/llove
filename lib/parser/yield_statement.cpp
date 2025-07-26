#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseYieldStatement(const bool inline_)
{
    Expect(TokenType_Symbol, "yield");
    if (!inline_ && SkipIf(TokenType_Other, ";"))
        return std::make_unique<YieldStatement>(nullptr);

    auto value = ParseExpression();

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<YieldStatement>(std::move(value));
}
