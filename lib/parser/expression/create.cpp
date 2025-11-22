#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseCreateExpression()
{
    auto token = Expect(TokenType_Symbol, "create");

    ExpressionPtr destination;
    if (SkipIf(TokenType_Other, "["))
    {
        destination = ParseExpression();
        Expect(TokenType_Other, "]");
    }

    auto type = ParseType();

    std::vector<ExpressionPtr> arguments;
    if (SkipIf(TokenType_Other, "("))
    {
        while (!At(TokenType_Other, ")"))
        {
            arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Other, ")"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, ")");
    }

    return std::make_unique<CreateExpression>(std::move(token.Loc), std::move(type), std::move(destination), std::move(arguments));
}
