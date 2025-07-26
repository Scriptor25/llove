#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseIfStatement(const bool inline_)
{
    Expect(TokenType_Symbol, "if");
    Expect(TokenType_Other, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Other, ")");
    auto then = ScopeStatement::Wrap(ParseStatement(inline_));

    StatementPtr else_;
    if (SkipIf(TokenType_Symbol, "else"))
        else_ = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<IfStatement>(std::move(condition), std::move(then), std::move(else_));
}
