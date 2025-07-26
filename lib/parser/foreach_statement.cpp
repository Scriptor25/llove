#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseForEachStatement(const bool inline_)
{
    Expect(TokenType_Sym, "foreach");
    Expect(TokenType_Otr, "(");

    auto mutable_ = SkipIf(TokenType_Sym, "mut");
    auto reference = SkipIf(TokenType_Opr, "&");
    auto name = Expect(TokenType_Sym).Value;

    Expect(TokenType_Otr, ":");

    auto range = ParseExpression();

    Expect(TokenType_Otr, ")");

    auto content = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<ForEachStatement>(
        mutable_,
        reference,
        std::move(name),
        std::move(range),
        std::move(content));
}
