#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseStatement(const bool is_inline)
{
    if (At(TokenType_Symbol, "break"))
        return ParseBreakStatement(is_inline);
    if (At(TokenType_Symbol, "continue"))
        return ParseContinueStatement(is_inline);
    if (At(TokenType_Symbol, "delete"))
        return ParseDeleteStatement(is_inline);
    if (At(TokenType_Symbol, "for"))
        return ParseForStatement(is_inline);
    if (At(TokenType_Symbol, "foreach"))
        return ParseForEachStatement(is_inline);
    if (At(TokenType_Symbol, "if"))
        return ParseIfStatement(is_inline);
    if (At(TokenType_Symbol, "let"))
        return ParseLetStatement(is_inline);
    if (At(TokenType_Symbol, "ret"))
        return ParseRetStatement(is_inline);
    if (At(TokenType_Other, "{"))
        return ParseScopeStatement();
    if (At(TokenType_Symbol, "switch"))
        return ParseSwitchStatement();
    if (At(TokenType_Symbol, "while"))
        return ParseWhileStatement(is_inline);

    auto expression = ParseExpression();
    if (is_inline)
        return expression;

    Expect(TokenType_Other, ";");
    return expression;
}
