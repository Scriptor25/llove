#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseSubscriptExpression(ExpressionPtr value)
{
    auto token = Expect(TokenType_Other, "[");
    auto index = ParseExpression();
    Expect(TokenType_Other, "]");

    return std::make_unique<SubscriptExpression>(std::move(token.Loc), std::move(value), std::move(index));
}
