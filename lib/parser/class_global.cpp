#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassGlobal()
{
    auto loc = Expect(TokenType_Symbol, "class").Loc;

    if (At(TokenType_Operator, "<"))
    {
        ParseClassTemplate();
        return nullptr;
    }

    auto name = Expect(TokenType_Symbol).Value;
    auto type = m_Types.GetClass(std::move(name));
    m_Types.Set(type->GetName(), type);

    if (SkipIf(TokenType_Other, ";"))
        return std::make_unique<ClassGlobal>(std::move(loc), std::move(type));

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

        ParseClassFunction(functions.emplace_back());
    }
    Expect(TokenType_Other, "}");

    return std::make_unique<ClassGlobal>(std::move(loc), std::move(type), std::move(fields), std::move(functions));
}

void llove::Parser::ParseClassField(ClassField &field)
{
    Expect(TokenType_Symbol, "let");
    field.Name = ParseField(field.Info, true);
    if (SkipIf(TokenType_Operator, "="))
    {
        field.Value = ParseExpression();
    }
    else if (SkipIf(TokenType_Other, "("))
    {
        while (!At(TokenType_Other, ")"))
        {
            field.Arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Other, ")"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, ")");
    }
    Expect(TokenType_Other, ";");
}

void llove::Parser::ParseClassFunction(ClassFunction &function)
{
    function.Expose = SkipIf(TokenType_Symbol, "expose");
    function.Implicit = SkipIf(TokenType_Symbol, "implicit");
    function.Mutable = SkipIf(TokenType_Symbol, "mut");
    function.Name = At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    Expect(TokenType_Other, "(");
    while (!At(TokenType_Other, ")"))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            function.VarArg = true;
            break;
        }

        auto &[info_, name_] = function.Parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    if (SkipIf(TokenType_Other, ":"))
        ParseField(function.Result, false, false);
    else
        function.Result.Type = m_Types.GetVoid();

    if (SkipIf(TokenType_Other, ";"))
        return;

    function.Content = ParseScopeStatement();
}
