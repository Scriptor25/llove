#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseForEachStatement(const bool inline_)
{
    auto loc = Expect(TokenType_Symbol, "foreach").Loc;
    Expect(TokenType_Other, "(");

    auto mutable_ = SkipIf(TokenType_Symbol, "mut");
    auto reference = SkipIf(TokenType_Operator, "&");
    auto name = Expect(TokenType_Symbol).Value;

    Expect(TokenType_Other, ":");

    auto range = ParseExpression();

    Expect(TokenType_Other, ")");

    auto content = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<ForEachStatement>(
        std::move(loc),
        mutable_,
        reference,
        std::move(name),
        std::move(range),
        std::move(content));
}
