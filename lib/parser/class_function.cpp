#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseClassFunction(ClassFunction &function, const bool require_content)
{
    function.Loc = m_Token.Loc;
    function.IsExposed = SkipIf(TokenType_Symbol, "expose");
    function.IsVirtual = SkipIf(TokenType_Symbol, "virtual");
    function.IsOverride = SkipIf(TokenType_Symbol, "override");
    function.IsImplicit = SkipIf(TokenType_Symbol, "implicit");
    function.IsMutable = SkipIf(TokenType_Symbol, "mut");
    function.Name = At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    function.Variadic = ParseParameterList(function.Parameters);

    if (SkipIf(TokenType_Other, ":"))
        ParseField(function.Result, false, true);
    else
        function.Result.SetType(m_Context.GetVoid());

    if (!require_content && SkipIf(TokenType_Other, ";"))
        return;

    function.Content = ParseScopeStatement();
}
