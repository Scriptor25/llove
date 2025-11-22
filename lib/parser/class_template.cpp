#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/template.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassTemplate(
    const bool is_export,
    std::string name)
{
    std::vector<std::pair<std::string, TemplateType::Ptr>> template_parameters;
    ParseTemplateParameterList(template_parameters);

    if (SkipIf(TokenType_Other, ";"))
    {
        m_Context.EmplaceClassTemplate(is_export, std::move(name), std::move(template_parameters));
        return;
    }

    auto& class_template = m_Context.PushClassTemplate(is_export, std::move(name), std::move(template_parameters), false);

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
    {
        if (At(TokenType_Symbol, "let"))
        {
            ParseClassMember(class_template.Members.emplace_back());
            continue;
        }

        ParseClassFunction(class_template.Functions.emplace_back(), true);
    }
    Expect(TokenType_Other, "}");

    m_Context.PopClassTemplate();
}
