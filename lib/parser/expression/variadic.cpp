#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseVariadicExpression(ExpressionPtr list)
{
    auto token = Expect(TokenType_Other, "{");
    auto type = ParseType();
    Expect(TokenType_Other, "}");

    return std::make_unique<VariadicExpression>(std::move(token.Loc), std::move(list), std::move(type));
}
