#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseMemberExpression(ExpressionPtr value)
{
    auto token = Expect(TokenType_Operator, ".", "::");
    auto member = Expect(TokenType_Symbol).Value;

    return std::make_unique<MemberExpression>(std::move(token.Loc), std::move(value), std::move(member), token.Value == "::");
}
