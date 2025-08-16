#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassFunction(ClassFunction &function, const bool require_content)
{
    function.Loc = m_Token.Loc;
    function.Expose = SkipIf(TokenType_Symbol, "expose");
    function.Implicit = SkipIf(TokenType_Symbol, "implicit");
    function.Mutable = SkipIf(TokenType_Symbol, "mut");
    function.Name = At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    function.VarArg = ParseParameterList("(", function.Parameters, ")");

    if (SkipIf(TokenType_Other, ":"))
        ParseField(function.Result, false, true);
    else
        function.Result.Type = m_Context.GetVoid();

    if (!require_content && SkipIf(TokenType_Other, ";"))
        return;

    function.Content = ParseScopeStatement();
}
