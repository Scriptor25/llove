#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassMember(ClassMember& member)
{
    Expect(TokenType_Symbol, "let");
    member.Name = ParseField(member.Info, true, true);
    Expect(TokenType_Other, ";");
}
