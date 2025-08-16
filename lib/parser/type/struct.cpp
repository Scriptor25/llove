#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseStructType()
{
    Expect(TokenType_Other, "{");

    std::vector<Parameter> fields;
    while (!At(TokenType_Other, "}"))
    {
        Field field;
        auto name = ParseField(field, true, true);

        fields.emplace_back(field, name);

        if (!At(TokenType_Other, "}"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, "}");

    return m_Context.GetStruct(std::move(fields));
}
