#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseTemplateType()
{
    Expect(TokenType_Operator, "<");

    std::vector<TypePtr> template_arguments;
    while (!At(TokenType_Operator, ">"))
    {
        template_arguments.emplace_back(ParseType());

        if (!At(TokenType_Operator, ">"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Operator, ">");

    auto name = Expect(TokenType_Symbol).Value;
    return m_Context.InstantiateTypeTemplate(std::move(name), std::move(template_arguments), false);
}
