#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/template.hpp>

llove::TypePtr llove::Parser::ParseTemplateType()
{
    Expect(TokenType_Operator, "<");

    std::vector<TypePtr> arguments;
    while (!At(TokenType_Operator, ">"))
    {
        arguments.push_back(ParseType());

        if (!At(TokenType_Operator, ">"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Operator, ">");

    const auto name = Expect(TokenType_Symbol).Value;

    const auto &instance = m_Context.InstantiateTemplate<TypeTemplateInstance>(name, arguments);
    return instance.GetType();
}
