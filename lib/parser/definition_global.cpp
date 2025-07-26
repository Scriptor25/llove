#include <llove/context.hpp>
#include <llove/parameter.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseDefinitionGlobal()
{
    auto interface = SkipIf(TokenType_Sym, "interface") || (Expect(TokenType_Sym, "define"), false);

    if (!interface && SkipIf(TokenType_Otr, ":"))
        return ParseClassDefinitionGlobal();

    auto name = !interface && At(TokenType_Opr) ? Skip().Value : Expect(TokenType_Sym).Value;

    std::vector<Parameter> parameters;
    auto vararg = false;

    Expect(TokenType_Otr, "(");
    while (!At(TokenType_Otr, ")"))
    {
        if (SkipIf(TokenType_Opr, "..."))
        {
            vararg = true;
            break;
        }

        auto &[info_, name_] = parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Otr, ")"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Otr, ")");

    Field result;
    if (SkipIf(TokenType_Otr, ":"))
        ParseField(result, false, false);
    else
        result.Type = m_Types.GetVoid();

    if (SkipIf(TokenType_Otr, ";"))
        return std::make_unique<DefinitionGlobal>(
            interface,
            std::move(name),
            std::move(parameters),
            vararg,
            std::move(result),
            nullptr);

    auto content = ParseScopeStatement();

    return std::make_unique<DefinitionGlobal>(
        interface,
        std::move(name),
        std::move(parameters),
        vararg,
        std::move(result),
        std::move(content));
}
