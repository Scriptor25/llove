#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseIfStatement(const bool inline_)
{
    Expect(TokenType_Sym, "if");
    Expect(TokenType_Otr, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Otr, ")");
    auto then = ScopeStatement::Wrap(ParseStatement(inline_));

    StatementPtr else_;
    if (SkipIf(TokenType_Sym, "else"))
        else_ = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<IfStatement>(std::move(condition), std::move(then), std::move(else_));
}
