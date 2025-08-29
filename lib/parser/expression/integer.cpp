#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseIntegerExpression()
{
    auto token = Expect(TokenType_Integer);

    TypePtr type;
    if (SkipIf(TokenType_Operator, ":"))
        type = ParseType();

    return std::make_unique<IntegerExpression>(std::move(token.Loc), token.IntegerValue, std::move(type));
}
