#include <llove/field.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseLetStatement(const bool inline_)
{
    auto loc = Expect(TokenType_Symbol, "let").Loc;

    Field info;
    auto name = ParseField(info, true, false);

    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments;

    if (SkipIf(TokenType_Operator, "="))
    {
        value = ParseExpression();
    }
    else if (SkipIf(TokenType_Other, "("))
    {
        while (!At(TokenType_Other, ")"))
        {
            arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Other, ")"))
                Expect(TokenType_Other, ",");
        }

        Expect(TokenType_Other, ")");
    }

    if (!inline_)
        Expect(TokenType_Other, ";");

    return std::make_unique<LetStatement>(
        std::move(loc),
        std::move(info),
        std::move(name),
        std::move(value),
        std::move(arguments));
}
