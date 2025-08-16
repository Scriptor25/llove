#include <llove/class_template.hpp>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassTemplate()
{
    std::vector<std::pair<std::string, TemplateType::Ptr>> parameters;

    Expect(TokenType_Operator, "<");
    while (!At(TokenType_Operator, ">"))
    {
        auto name = Expect(TokenType_Symbol).Value;
        parameters.emplace_back(name, std::make_shared<TemplateType>(name));

        if (!At(TokenType_Operator, ">"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Operator, ">");

    auto name = Expect(TokenType_Symbol).Value;

    if (SkipIf(TokenType_Other, ";"))
    {
        m_Context.EmplaceTemplate(std::move(name), std::move(parameters));
        return;
    }

    auto &template_ = m_Context.PushTemplate(std::move(name), std::move(parameters));

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
    {
        if (At(TokenType_Symbol, "let"))
        {
            ParseClassField(template_.Fields.emplace_back());
            continue;
        }

        ParseClassFunction(template_.Functions.emplace_back(), true);
    }
    Expect(TokenType_Other, "}");

    m_Context.PopTemplate();
}
