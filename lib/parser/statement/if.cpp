#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseIfStatement(const bool is_inline)
{
    auto loc = Expect(TokenType_Symbol, "if").Loc;
    Expect(TokenType_Other, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Other, ")");
    auto then = ScopeStatement::Wrap(ParseStatement(is_inline));

    StatementPtr else_;
    if (SkipIf(TokenType_Symbol, "else"))
        else_ = ScopeStatement::Wrap(ParseStatement(is_inline));

    return std::make_unique<IfStatement>(std::move(loc), std::move(condition), std::move(then), std::move(else_));
}
