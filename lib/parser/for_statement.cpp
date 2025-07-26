#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseForStatement(const bool inline_)
{
    Expect(TokenType_Symbol, "for");
    Expect(TokenType_Other, "(");

    StatementPtr prefix, suffix;
    ExpressionPtr condition;

    if (!SkipIf(TokenType_Other, ";"))
    {
        prefix = ParseStatement(true);
        Expect(TokenType_Other, ";");
    }

    if (!SkipIf(TokenType_Other, ";"))
    {
        condition = ParseExpression();
        Expect(TokenType_Other, ";");
    }

    if (!SkipIf(TokenType_Other, ")"))
    {
        suffix = ParseStatement(true);
        Expect(TokenType_Other, ")");
    }

    auto content = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<ForStatement>(
        std::move(prefix),
        std::move(suffix),
        std::move(condition),
        std::move(content));
}
