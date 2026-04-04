#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::Location llove::Parser::ParseArgumentList(std::vector<ExpressionPtr> &arguments)
{
    return ParseList<ExpressionPtr>(
        arguments,
        [&](ExpressionPtr &element)
        {
            element = ParseExpression();
        },
        TokenType_Other,
        "(",
        TokenType_Other,
        ")");
}
