#include <llove/field.hpp>
#include <llove/parser.hpp>

std::string llove::Parser::ParseField(Field &field, const bool require_name, bool require_type)
{
    field.SetIsMutable(SkipIf(TokenType_Symbol, "mut"));
    field.SetIsReference(SkipIf(TokenType_Operator, "&"));

    std::string name;
    if (require_name)
    {
        name = Expect(TokenType_Symbol).Value;
        require_type |= SkipIf(TokenType_Operator, ":");
    }

    if (require_type)
        field.SetType(ParseType());

    return name;
}
