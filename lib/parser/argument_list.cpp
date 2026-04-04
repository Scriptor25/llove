#include <llove/parser.hpp>
#include <llove/tree.hpp>

using namespace std::placeholders;

llove::Location llove::Parser::ParseArgumentList(std::vector<ExpressionPtr> &arguments)
{
    return ParseList<ExpressionPtr>(
        arguments,
        std::bind(&Parser::ParseExpressionElement, this, _1),
        TokenType_Other,
        "(",
        TokenType_Other,
        ")");
}
