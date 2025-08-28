#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassGlobal(const bool is_export)
{
    auto loc = Expect(TokenType_Symbol, "class").Loc;
    auto name = Expect(TokenType_Symbol).Value;

    if (At(TokenType_Operator, "<"))
    {
        ParseClassTemplate(is_export, std::move(name));
        return nullptr;
    }

    auto type = m_Context.GetClass(std::move(name));
    m_Context.SetNamed(type->GetName(), type);

    if (SkipIf(TokenType_Other, ";"))
        return std::make_unique<ClassGlobal>(std::move(loc), is_export, std::move(type));

    std::vector<ClassMember> members;
    std::vector<ClassFunction> functions;

    ClassType::Ptr base_type;
    if (SkipIf(TokenType_Other, ":"))
        base_type = As<ClassType>(ParseType());

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
    {
        if (At(TokenType_Symbol, "let"))
        {
            ParseClassMember(members.emplace_back());
            continue;
        }

        ParseClassFunction(functions.emplace_back(), false);
    }
    Expect(TokenType_Other, "}");

    return std::make_unique<ClassGlobal>(
        std::move(loc),
        is_export,
        std::move(type),
        std::move(base_type),
        std::move(members),
        std::move(functions));
}
