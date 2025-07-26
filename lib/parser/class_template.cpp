#include <llove/class_template.hpp>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassTemplate()
{
    std::vector<std::pair<std::string, TemplateType::Ptr>> parameters;

    Expect(TokenType_Opr, "<");
    while (!At(TokenType_Opr, ">"))
    {
        auto name = Expect(TokenType_Sym).Value;
        parameters.emplace_back(name, std::make_shared<TemplateType>(name));

        if (!At(TokenType_Opr, ">"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Opr, ">");

    auto name = Expect(TokenType_Sym).Value;

    if (SkipIf(TokenType_Otr, ";"))
    {
        m_Types.EmplaceTemplate(std::move(name), std::move(parameters));
        return;
    }

    auto &template_ = m_Types.PushTemplate(std::move(name), std::move(parameters));

    Expect(TokenType_Otr, "{");
    while (!At(TokenType_Otr, "}"))
    {
        if (At(TokenType_Sym, "let"))
        {
            ParseClassField(template_.Fields.emplace_back());
            continue;
        }

        ParseClassFunction(template_.Functions.emplace_back());
    }
    Expect(TokenType_Otr, "}");

    m_Types.PopTemplate();
}
