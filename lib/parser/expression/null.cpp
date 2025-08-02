#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseNullExpression()
{
    auto token = Expect(TokenType_Symbol, "null");

    TypePtr type;
    if (SkipIf(TokenType_Other, ":"))
        type = ParseType();

    return std::make_unique<NullExpression>(std::move(token.Loc), type);
}
