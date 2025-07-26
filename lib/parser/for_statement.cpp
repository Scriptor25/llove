#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseForStatement(const bool inline_)
{
    Expect(TokenType_Sym, "for");
    Expect(TokenType_Otr, "(");

    StatementPtr prefix, suffix;
    ExpressionPtr condition;

    if (!SkipIf(TokenType_Otr, ";"))
    {
        prefix = ParseStatement(true);
        Expect(TokenType_Otr, ";");
    }

    if (!SkipIf(TokenType_Otr, ";"))
    {
        condition = ParseExpression();
        Expect(TokenType_Otr, ";");
    }

    if (!SkipIf(TokenType_Otr, ")"))
    {
        suffix = ParseStatement(true);
        Expect(TokenType_Otr, ")");
    }

    auto content = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<ForStatement>(
        std::move(prefix),
        std::move(suffix),
        std::move(condition),
        std::move(content));
}
