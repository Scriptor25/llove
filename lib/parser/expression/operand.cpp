#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseOperandExpression()
{
    auto expression = ParsePrimaryExpression();

    while (true)
    {
        if (At(TokenType_Other, "("))
        {
            expression = ParseCallExpression(std::move(expression));
            continue;
        }

        if (At(TokenType_Operator, "."))
        {
            expression = ParseMemberExpression(std::move(expression));
            continue;
        }

        if (At(TokenType_Operator, ".."))
        {
            expression = ParseRangeExpression(std::move(expression));
            continue;
        }

        if (At(TokenType_Other, "["))
        {
            expression = ParseSubscriptExpression(std::move(expression));
            continue;
        }

        if (At(TokenType_Operator, "++", "--"))
        {
            expression = ParseUnaryExpression(std::move(expression));
            continue;
        }

        if (At(TokenType_Other, "{"))
        {
            expression = ParseVariadicExpression(std::move(expression));
            continue;
        }

        if (At(TokenType_Symbol, "as"))
        {
            expression = ParseCastExpression(std::move(expression));
            continue;
        }

        return expression;
    }
}
