#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassMember(ClassMember &member)
{
    Expect(TokenType_Symbol, "let");
    member.Name = ParseField(member.Info, true, true);
    if (SkipIf(TokenType_Operator, "="))
    {
        member.Value = ParseExpression();
    }
    else if (SkipIf(TokenType_Other, "("))
    {
        while (!At(TokenType_Other, ")"))
        {
            member.Arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Other, ")"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, ")");
    }
    Expect(TokenType_Other, ";");
}
