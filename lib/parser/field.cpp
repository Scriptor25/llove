#include <llove/field.hpp>
#include <llove/parser.hpp>

std::string llove::Parser::ParseField(Field &field, const bool require_name, const bool allow_name)
{
    field.Mutable = SkipIf(TokenType_Symbol, "mut");
    field.Reference = SkipIf(TokenType_Operator, "&");

    auto allow_type = !allow_name;

    std::string name;
    if (allow_name && (require_name || At(TokenType_Symbol)))
    {
        name = Expect(TokenType_Symbol).Value;
        allow_type = SkipIf(TokenType_Other, ":");
    }

    if (allow_type)
        field.Type = ParseType();

    return name;
}
