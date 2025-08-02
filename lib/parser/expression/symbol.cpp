#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseSymbolExpression()
{
    auto token = Expect(TokenType_Symbol);
    return std::make_unique<SymbolExpression>(std::move(token.Loc), std::move(token.Value));
}
