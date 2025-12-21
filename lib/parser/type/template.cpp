#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/parser.hpp>
#include <llove/template.hpp>

llove::TypePtr llove::Parser::ParseTemplateType()
{
    Expect(TokenType_Operator, "<");

    std::vector<TypePtr> arguments;
    while (!At(TokenType_Operator, ">"))
    {
        arguments.emplace_back(ParseType());

        if (!At(TokenType_Operator, ">"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Operator, ">");

    auto name = Expect(TokenType_Symbol).Value;

    auto& instance = m_Context.InstantiateTemplate<TypeTemplateInstance>(name, std::move(arguments));
    return instance.GetType();
}
