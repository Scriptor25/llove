#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParsePrimaryExpression()
{
    if (SkipIf(TokenType_Other, "("))
    {
        auto expression = ParseExpression();
        Expect(TokenType_Other, ")");
        return expression;
    }

    if (At(TokenType_Integer))
        return ParseIntegerExpression();

    if (At(TokenType_Float))
        return ParseFloatExpression();

    if (At(TokenType_String))
        return ParseStringExpression();

    if (At(TokenType_Operator, "-", "!", "~", "++", "--", "*", "&", "$"))
        return ParseUnaryExpression();

    if (At(TokenType_Other, "["))
        return ParseArrayExpression();

    if (At(TokenType_Other, "{"))
        return ParseStructExpression();

    if (At(TokenType_Symbol, "null"))
        return ParseNullExpression();

    if (At(TokenType_Symbol, "create"))
        return ParseCreateExpression();

    if (At(TokenType_Symbol, "sizeof"))
        return ParseSizeofExpression();

    if (At(TokenType_Symbol))
        return ParseSymbolExpression();

    Error("unable to parse expression from {} : '{}'", m_Token.Type, m_Token.Value);
}
