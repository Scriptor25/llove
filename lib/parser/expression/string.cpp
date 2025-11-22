#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseStringExpression()
{
    auto token = Expect(TokenType_String);
    return std::make_unique<StringExpression>(
        std::move(token.Loc),
        std::move(token.Value));
}
