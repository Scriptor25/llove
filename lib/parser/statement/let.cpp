#include <llove/field.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseLetStatement(const bool is_inline)
{
    auto token = Expect(TokenType_Symbol, "let");

    Field info;
    auto name = ParseField(info, true, false);

    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments;

    if (SkipIf(TokenType_Operator, "=") || (!info.HasType() && (Expect(TokenType_Operator, "="), true)))
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

    if (!is_inline)
        Expect(TokenType_Other, ";");

    return std::make_unique<LetStatement>(std::move(token.Loc), std::move(info), std::move(name), std::move(value), std::move(arguments));
}
