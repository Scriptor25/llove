#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseStatement(const bool inline_)
{
    if (At(TokenType_Otr, "{"))
        return ParseScopeStatement();
    if (At(TokenType_Sym, "for"))
        return ParseForStatement(inline_);
    if (At(TokenType_Sym, "foreach"))
        return ParseForEachStatement(inline_);
    if (At(TokenType_Sym, "if"))
        return ParseIfStatement(inline_);
    if (At(TokenType_Sym, "let"))
        return ParseLetStatement(inline_);
    if (At(TokenType_Sym, "yield"))
        return ParseYieldStatement(inline_);

    auto expression = ParseExpression();
    if (inline_)
        return expression;

    Expect(TokenType_Otr, ";");
    return expression;
}
