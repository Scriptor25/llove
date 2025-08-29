#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseFloatExpression()
{
    auto token = Expect(TokenType_Float);

    TypePtr type;
    if (SkipIf(TokenType_Operator, ":"))
        type = ParseType();

    return std::make_unique<FloatExpression>(std::move(token.Loc), token.FloatValue, std::move(type));
}
