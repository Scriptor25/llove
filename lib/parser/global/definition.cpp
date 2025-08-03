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
    auto vararg = ParseParameterList("(", parameters, ")");

    Field result;
    if (SkipIf(TokenType_Other, ":"))
        ParseField(result, false, true);
    else
        result.Type = m_Types.GetVoid();

    StatementPtr content;
    if (!SkipIf(TokenType_Other, ";"))
        content = ParseScopeStatement();

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
