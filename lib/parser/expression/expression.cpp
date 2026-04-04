#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseExpression()
{
    return ParseBinaryExpression();
}
