#include <llove/context.hpp>
#include <llove/parameter.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseFunctionGlobal(
    bool is_template,
    const bool is_export)
{
    auto loc = m_Token.Loc;
    auto is_interface = SkipIf(TokenType_Symbol, "interface") || (Expect(TokenType_Symbol, "function"), false);

    if (!is_template && !is_export && !is_interface && SkipIf(TokenType_Operator, ":"))
        return ParseClassFunctionGlobal(std::move(loc));

    auto is_implicit = !is_interface && SkipIf(TokenType_Symbol, "implicit");

    std::string name;
    bool is_operator;

    if (is_implicit)
    {
        name = Expect(TokenType_Symbol, "create", "cast").Value;
        is_operator = false;
    }
    else if (!is_interface && At(TokenType_Operator))
    {
        name = Skip().Value;
        is_operator = true;
    }
    else
    {
        name = Expect(TokenType_Symbol).Value;
        is_operator = false;
    }

    std::vector<Parameter> parameters;
    std::pair<bool, std::string> variadic;
    ParseParameterList(parameters, variadic);

    Field result;
    if (SkipIf(TokenType_Operator, ":"))
    {
        ParseField(result, false, true);
    }
    else
    {
        result.SetType(m_Context.GetVoid());
    }

    StatementPtr content;
    if (!SkipIf(TokenType_Other, ";"))
    {
        content = ParseScopeStatement();
    }

    return std::make_unique<FunctionGlobal>(
        std::move(loc),
        is_export,
        is_interface,
        is_implicit,
        is_operator,
        std::move(name),
        std::move(parameters),
        variadic,
        std::move(result),
        std::move(content));
}
