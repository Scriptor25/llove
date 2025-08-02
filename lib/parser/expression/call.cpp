#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseCallExpression(ExpressionPtr callee)
{
    auto token = Expect(TokenType_Other, "(");

    std::vector<ExpressionPtr> arguments;
    while (!At(TokenType_Other, ")"))
    {
        arguments.emplace_back(ParseExpression());

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    return std::make_unique<CallExpression>(std::move(token.Loc), std::move(callee), std::move(arguments));
}
