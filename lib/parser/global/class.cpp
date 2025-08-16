#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassGlobal(const bool export_)
{
    auto loc = Expect(TokenType_Symbol, "class").Loc;

    if (At(TokenType_Operator, "<"))
    {
        ParseClassTemplate();
        return nullptr;
    }

    auto name = Expect(TokenType_Symbol).Value;
    auto type = m_Context.GetClass(std::move(name));
    m_Context.SetNamed(type->GetName(), type);

    if (SkipIf(TokenType_Other, ";"))
        return std::make_unique<ClassGlobal>(std::move(loc), export_, std::move(type));

    std::vector<ClassField> fields;
    std::vector<ClassFunction> functions;

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
    {
        if (At(TokenType_Symbol, "let"))
        {
            ParseClassField(fields.emplace_back());
            continue;
        }

        ParseClassFunction(functions.emplace_back(), false);
    }
    Expect(TokenType_Other, "}");

    return std::make_unique<ClassGlobal>(std::move(loc), export_, std::move(type), std::move(fields), std::move(functions));
}
