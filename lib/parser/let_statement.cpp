#include <llove/field.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseLetStatement(const bool inline_)
{
    Expect(TokenType_Sym, "let");

    Field info;
    auto name = ParseField(info, true);

    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments;

    if (SkipIf(TokenType_Opr, "="))
    {
        value = ParseExpression();
    }
    else if (SkipIf(TokenType_Otr, "("))
    {
        while (!At(TokenType_Otr, ")"))
        {
            arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Otr, ")"))
                Expect(TokenType_Otr, ",");
        }

        Expect(TokenType_Otr, ")");
    }

    if (!inline_)
        Expect(TokenType_Otr, ";");

    return std::make_unique<LetStatement>(std::move(info), std::move(name), std::move(value), std::move(arguments));
}
