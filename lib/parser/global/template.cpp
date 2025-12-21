#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>
#include <memory>

llove::GlobalPtr llove::Parser::ParseTemplateGlobal(bool is_export)
{
    auto loc = Expect(TokenType_Symbol, "template").Loc;

    std::vector<TemplateParameter> parameters;
    ParseTemplateParameterList(parameters);

    m_Context.PushTemplate(parameters);

    auto content = ParseGlobal(true);

    m_Context.PopTemplate();

    return std::make_unique<TemplateGlobal>(std::move(loc), is_export, std::move(parameters), std::move(content));
}
