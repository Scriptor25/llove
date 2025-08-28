#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseTemplateCallExpression()
{
    std::vector<TypePtr> type_arguments;
    std::vector<ExpressionPtr> arguments;

    auto token = Expect(TokenType_Operator, "<");

    while (!At(TokenType_Operator, ">"))
    {
        type_arguments.emplace_back(ParseType());

        if (!At(TokenType_Operator, ">"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Operator, ">");

    auto callee = Expect(TokenType_Symbol).Value;

    Expect(TokenType_Other, "(");
    while (!At(TokenType_Other, ")"))
    {
        arguments.emplace_back(ParseExpression());

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    return std::make_unique<TemplateCallExpression>(
        std::move(token.Loc),
        std::move(type_arguments),
        std::move(callee),
        std::move(arguments));
}
