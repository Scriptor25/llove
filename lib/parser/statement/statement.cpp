#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseStatement(const bool inline_)
{
    if (At(TokenType_Symbol, "delete"))
        return ParseDeleteStatement(inline_);
    if (At(TokenType_Symbol, "for"))
        return ParseForStatement(inline_);
    if (At(TokenType_Symbol, "foreach"))
        return ParseForEachStatement(inline_);
    if (At(TokenType_Symbol, "if"))
        return ParseIfStatement(inline_);
    if (At(TokenType_Symbol, "let"))
        return ParseLetStatement(inline_);
    if (At(TokenType_Other, "{"))
        return ParseScopeStatement();
    if (At(TokenType_Symbol, "while"))
        return ParseWhileStatement(inline_);
    if (At(TokenType_Symbol, "yield"))
        return ParseYieldStatement(inline_);

    auto expression = ParseExpression();
    if (inline_)
        return expression;

    Expect(TokenType_Other, ";");
    return expression;
}
