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
        return ParseLambdaExpression();

    if (At(TokenType_Other, "{"))
        return ParseInitializerExpression();

    if (At(TokenType_Operator, "<"))
        return ParseTemplateCallExpression();

    if (At(TokenType_Symbol, "null"))
        return ParseNullExpression();

    if (At(TokenType_Symbol, "create"))
        return ParseCreateExpression();

    if (At(TokenType_Symbol, "sizeof"))
        return ParseSizeofExpression();

    if (At(TokenType_Symbol, "switch"))
        return ParseSwitchExpression();

    if (At(TokenType_Symbol, "false", "true"))
    {
        auto token = Skip();
        return std::make_unique<IntegerExpression>(
            std::move(token.Loc),
            token.Value == "true",
            m_Context.GetBoolean());
    }

    if (At(TokenType_Symbol, "inline"))
        return ParseInlineExpression();

    if (At(TokenType_Symbol))
        return ParseSymbolExpression();

    Error(m_Token.Loc, "unable to parse expression from {} : '{}'", m_Token.Type, m_Token.Value);
}
