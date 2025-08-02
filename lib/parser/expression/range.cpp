#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseRangeExpression(ExpressionPtr begin)
{
    auto token = Expect(TokenType_Operator, "..");
    auto end = ParsePrimaryExpression();

    return std::make_unique<RangeExpression>(std::move(token.Loc), std::move(begin), std::move(end));
}
