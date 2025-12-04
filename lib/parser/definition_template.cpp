#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>

void llove::Parser::ParseDefinitionTemplate(
    const bool is_export,
    Location loc,
    const bool is_implicit,
    std::string name)
{
    std::vector<std::pair<std::string, TemplateType::Ptr>> template_parameters;
    ParseTemplateParameterList(template_parameters);

    auto& definition_template = m_Context.PushDefinitionTemplate(is_export, is_implicit, std::move(loc), std::move(name), std::move(template_parameters), false);

    ParseParameterList(definition_template.Parameters, definition_template.Variadic);

    if (SkipIf(TokenType_Operator, ":"))
        ParseField(definition_template.Result, false, true);
    else
        definition_template.Result.SetType(m_Context.GetVoid());

    definition_template.Content = ParseScopeStatement();

    m_Context.PopDefinitionTemplate();
}
