#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseClassType()
{
    Expect(TokenType_Symbol, "class");

    if (SkipIf(TokenType_Operator, "<"))
    {
        std::vector<TypePtr> template_arguments;
        while (!At(TokenType_Operator, ">"))
        {
            template_arguments.emplace_back(ParseType());

            if (!At(TokenType_Operator, ">"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Operator, ">");

        auto name = Expect(TokenType_Symbol).Value;
        return m_Context.InstantiateTemplateClass(std::move(name), template_arguments);
    }

    auto name = Expect(TokenType_Symbol).Value;
    return m_Context.GetClass(std::move(name));
}
