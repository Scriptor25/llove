#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>

llove::GlobalPtr llove::Parser::ParseClassGlobal(
    const bool is_template,
    const bool is_export)
{
    auto loc = Expect(TokenType_Symbol, "class").Loc;

    auto name = Expect(TokenType_Symbol).Value;
    auto type = m_Context.GetClass(std::move(name));

    if (is_template)
    {
        m_Context.CreateTemplate(
            type->GetName(),
            std::make_unique<TypeTemplateInstance>(m_Context.GetInstance(type->GetName())));
    }
    else
    {
        m_Context.SetNamed(type->GetName(), type);

        if (SkipIf(TokenType_Other, ";"))
            return std::make_unique<ClassGlobal>(std::move(loc), is_export, std::move(type));
    }

    std::vector<ClassMember> members;
    std::vector<ClassFunction> functions;

    ClassType::Ptr base_type;
    if (SkipIf(TokenType_Operator, ":"))
        base_type = As<ClassType>(ParseType());

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
    {
        if (At(TokenType_Symbol, "let"))
        {
            ParseClassMember(members.emplace_back());
            continue;
        }

        ParseClassFunction(functions.emplace_back(), is_template);
    }
    Expect(TokenType_Other, "}");

    return std::make_unique<ClassGlobal>(std::move(loc), is_export, std::move(type), std::move(base_type), std::move(members), std::move(functions));
}
