#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseSizeofExpression()
{
    auto token = Expect(TokenType_Symbol, "sizeof");
    auto type = ParseType();

    return std::make_unique<SizeofExpression>(std::move(token.Loc), std::move(type));
}
