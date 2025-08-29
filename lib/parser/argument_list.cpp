#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::Location llove::Parser::ParseArgumentList(std::vector<ExpressionPtr> &arguments)
{
    return ParseList<ExpressionPtr>(
        arguments,
        [this](auto &element)
        {
            element = ParseExpression();
        },
        TokenType_Other,
        "(",
        TokenType_Other,
        ")");
}
