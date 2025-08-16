#include <llove/context.hpp>
#include <llove/parameter.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseDefinitionGlobal(const bool export_)
{
    auto loc = m_Token.Loc;
    auto interface = SkipIf(TokenType_Symbol, "interface") || (Expect(TokenType_Symbol, "define"), false);

    if (!export_ && !interface && SkipIf(TokenType_Other, ":"))
        return ParseClassDefinitionGlobal(std::move(loc));

    auto implicit = !interface && SkipIf(TokenType_Symbol, "implicit");

    std::string name;
    if (implicit)
        name = Expect(TokenType_Symbol, "create", "cast").Value;
    else if (!interface && At(TokenType_Operator))
        name = Skip().Value;
    else
        name = Expect(TokenType_Symbol).Value;

    std::vector<Parameter> parameters;
    auto vararg = ParseParameterList("(", parameters, ")");

    Field result;
    if (SkipIf(TokenType_Other, ":"))
        ParseField(result, false, true);
    else
        result.Type = m_Context.GetVoid();

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
