#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

using namespace std::placeholders;

void llove::Parser::ParseClassFunction(ClassFunction &function, const bool require_content)
{
    static const std::set<std::string_view> no_result
    {
        "create",
        "delete",
    };

    function.Loc = m_Token.Loc;
    function.IsPublic = SkipIf(TokenType_Symbol, "public");
    function.IsVirtual = SkipIf(TokenType_Symbol, "virtual");
    function.IsOverride = SkipIf(TokenType_Symbol, "override");
    function.IsImplicit = SkipIf(TokenType_Symbol, "implicit");
    function.IsMutable = SkipIf(TokenType_Symbol, "mut");

    if (At(TokenType_Symbol) || At(TokenType_Operator))
        function.Name = Skip().Value;
    else if (SkipIf(TokenType_Other, "("))
    {
        Expect(TokenType_Other, ")");
        function.Name = "()";
    }
    else if (SkipIf(TokenType_Other, "["))
    {
        Expect(TokenType_Other, "]");
        function.Name = "[]";
    }

    ParseParameterList(function.Parameters, function.Variadic);

    if (!no_result.contains(function.Name) && SkipIf(TokenType_Operator, ":"))
        ParseField(function.Result, false, true);
    else
        function.Result.SetType(m_Context.GetVoid());

    if (function.Name == "create" && At(TokenType_Other, "["))
        ParseList<Initializer>(
            function.Initializers,
            std::bind(&Parser::ParseInitializer, this, _1),
            TokenType_Other,
            "[",
            TokenType_Other,
            "]");

    if (function.Initializers.empty() && !require_content && SkipIf(TokenType_Other, ";"))
        return;

    function.Content = ParseScopeStatement();
}
