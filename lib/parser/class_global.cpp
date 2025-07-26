#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassGlobal()
{
    Expect(TokenType_Sym, "class");

    if (At(TokenType_Opr, "<"))
    {
        ParseClassTemplate();
        return nullptr;
    }

    auto name = Expect(TokenType_Sym).Value;
    auto type = m_Types.GetClass(std::move(name));
    m_Types.Set(type->GetName(), type);

    if (SkipIf(TokenType_Otr, ";"))
        return std::make_unique<ClassGlobal>(std::move(type));

    std::vector<ClassField> fields;
    std::vector<ClassFunction> functions;

    Expect(TokenType_Otr, "{");
    while (!At(TokenType_Otr, "}"))
    {
        if (At(TokenType_Sym, "let"))
        {
            ParseClassField(fields.emplace_back());
            continue;
        }

        ParseClassFunction(functions.emplace_back());
    }
    Expect(TokenType_Otr, "}");

    return std::make_unique<ClassGlobal>(std::move(type), std::move(fields), std::move(functions));
}

void llove::Parser::ParseClassField(ClassField &field)
{
    Expect(TokenType_Sym, "let");
    field.Name = ParseField(field.Info, true);
    if (SkipIf(TokenType_Opr, "="))
    {
        field.Value = ParseExpression();
    }
    else if (SkipIf(TokenType_Otr, "("))
    {
        while (!At(TokenType_Otr, ")"))
        {
            field.Arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Otr, ")"))
                Expect(TokenType_Otr, ",");
        }
        Expect(TokenType_Otr, ")");
    }
    Expect(TokenType_Otr, ";");
}

void llove::Parser::ParseClassFunction(ClassFunction &function)
{
    function.Expose = SkipIf(TokenType_Sym, "expose");
    function.Mutable = SkipIf(TokenType_Sym, "mut");
    function.Name = At(TokenType_Opr) ? Skip().Value : Expect(TokenType_Sym).Value;

    Expect(TokenType_Otr, "(");
    while (!At(TokenType_Otr, ")"))
    {
        if (SkipIf(TokenType_Opr, "..."))
        {
            function.VarArg = true;
            break;
        }

        auto &[info_, name_] = function.Parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Otr, ")"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Otr, ")");

    if (SkipIf(TokenType_Otr, ":"))
        ParseField(function.Result, false, false);
    else
        function.Result.Type = m_Types.GetVoid();

    if (SkipIf(TokenType_Otr, ";"))
        return;

    function.Content = ParseScopeStatement();
}
