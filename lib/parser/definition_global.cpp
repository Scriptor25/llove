#include <llove/context.hpp>
#include <llove/parameter.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseDefinitionGlobal()
{
    auto loc = m_Token.Loc;
    auto interface = SkipIf(TokenType_Symbol, "interface") || (Expect(TokenType_Symbol, "define"), false);

    if (!interface && SkipIf(TokenType_Other, ":"))
        return ParseClassDefinitionGlobal(std::move(loc));

    auto implicit = SkipIf(TokenType_Symbol, "implicit");

    auto name = !interface && At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    std::vector<Parameter> parameters;
    auto vararg = false;

    Expect(TokenType_Other, "(");
    while (!At(TokenType_Other, ")"))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            vararg = true;
            break;
        }

        auto &[info_, name_] = parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    Field result;
    if (SkipIf(TokenType_Other, ":"))
        ParseField(result, false, false);
    else
        result.Type = m_Types.GetVoid();

    if (SkipIf(TokenType_Other, ";"))
        return std::make_unique<DefinitionGlobal>(
            std::move(loc),
            interface,
            implicit,
            std::move(name),
            std::move(parameters),
            vararg,
            std::move(result),
            nullptr);

    auto content = ParseScopeStatement();

    return std::make_unique<DefinitionGlobal>(
        std::move(loc),
        interface,
        implicit,
        std::move(name),
        std::move(parameters),
        vararg,
        std::move(result),
        std::move(content));
}
