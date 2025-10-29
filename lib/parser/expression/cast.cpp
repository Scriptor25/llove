#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseCastExpression(ExpressionPtr value)
{
    auto token = Expect(TokenType_Symbol, "as");
    auto type = ParseType();

    return std::make_unique<CastExpression>(std::move(token.Loc), std::move(value), std::move(type));
}
