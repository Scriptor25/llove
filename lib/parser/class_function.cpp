#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassFunction(ClassFunction &function, const bool require_content)
{
    static const std::set<std::string_view> no_result
    {
        "create",
        "delete",
    };

    function.Loc = m_Token.Loc;
    function.IsExposed = SkipIf(TokenType_Symbol, "expose");
    function.IsVirtual = SkipIf(TokenType_Symbol, "virtual");
    function.IsOverride = SkipIf(TokenType_Symbol, "override");
    function.IsImplicit = SkipIf(TokenType_Symbol, "implicit");
    function.IsMutable = SkipIf(TokenType_Symbol, "mut");
    function.Name = At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    ParseParameterList(function.Parameters, function.Variadic);

    if (!no_result.contains(function.Name) && SkipIf(TokenType_Operator, ":"))
        ParseField(function.Result, false, true);
    else
        function.Result.SetType(m_Context.GetVoid());

    if (function.Name == "create" && At(TokenType_Other, "["))
        ParseList<Initializer>(
            function.Initializers,
            [this](auto &element)
            {
                ParseInitializer(element);
            },
            TokenType_Other,
            "[",
            TokenType_Other,
            "]");

    if (function.Initializers.empty() && !require_content && SkipIf(TokenType_Other, ";"))
        return;

    function.Content = ParseScopeStatement();
}
