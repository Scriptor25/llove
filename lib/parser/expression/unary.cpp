#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseUnaryExpression()
{
    auto token = Expect(TokenType_Operator, "-", "!", "~", "++", "--", "*", "&", "$");
    auto operand = ParseOperandExpression();

    return std::make_unique<UnaryExpression>(
        std::move(token.Loc),
        std::move(token.Value),
        std::move(operand),
        false);
}

llove::ExpressionPtr llove::Parser::ParseUnaryExpression(ExpressionPtr operand)
{
    auto token = Expect(TokenType_Operator, "++", "--");

    return std::make_unique<UnaryExpression>(
        std::move(token.Loc),
        std::move(token.Value),
        std::move(operand),
        true);
}
