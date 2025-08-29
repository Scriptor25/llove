#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseCallExpression(ExpressionPtr callee)
{
    std::vector<ExpressionPtr> arguments;
    auto loc = ParseArgumentList(arguments);

    return std::make_unique<CallExpression>(std::move(loc), std::move(callee), std::move(arguments));
}
