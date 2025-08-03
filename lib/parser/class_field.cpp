#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassField(ClassField &field)
{
    Expect(TokenType_Symbol, "let");
    field.Name = ParseField(field.Info, true, true);
    if (SkipIf(TokenType_Operator, "="))
    {
        field.Value = ParseExpression();
    }
    else if (SkipIf(TokenType_Other, "("))
    {
        while (!At(TokenType_Other, ")"))
        {
            field.Arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Other, ")"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, ")");
    }
    Expect(TokenType_Other, ";");
}
