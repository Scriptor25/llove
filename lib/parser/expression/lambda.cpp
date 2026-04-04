#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseLambdaExpression()
{
    auto loc = Expect(TokenType_Other, "[").Loc;

    std::optional<Capture> default_capture;
    std::map<std::string, Capture> captures;

    while (!At(TokenType_Other, "]"))
    {
        auto cap_loc = m_Token.Loc;

        const auto is_mutable = SkipIf(TokenType_Symbol, "mut");
        const auto is_reference = SkipIf(TokenType_Operator, "&");

        if (!default_capture && !At(TokenType_Symbol))
        {
            if (is_reference)
                default_capture = { std::move(cap_loc), is_mutable, true };
            else
            {
                Expect(TokenType_Operator, "=");
                default_capture = { std::move(cap_loc), is_mutable, false };
            }
        }
        else
        {
            auto name = Expect(TokenType_Symbol).Value;
            captures[name] = { std::move(cap_loc), is_mutable, is_reference };
        }

        if (!At(TokenType_Other, "]"))
            Expect(TokenType_Other, ",");
    }

    Expect(TokenType_Other, "]");

    std::vector<Parameter> parameters;
    Variadic variadic;
    ParseParameterList(parameters, variadic);

    Field result;
    if (SkipIf(TokenType_Operator, ":"))
        ParseField(result, false, true);
    else
        result.SetType(m_Context.GetVoid());

    StatementPtr content;
    if (At(TokenType_Operator, "->"))
    {
        auto ret_loc = Skip().Loc;

        auto expression = ParseExpression();

        if (result.GetType()->IsVoid())
            content = std::move(expression);
        else
            content = std::make_unique<RetStatement>(std::move(ret_loc), std::move(expression));
    }
    else
        content = ParseScopeStatement();

    return std::make_unique<LambdaExpression>(
        std::move(loc),
        std::move(default_capture),
        std::move(captures),
        std::move(parameters),
        std::move(variadic),
        std::move(result),
        std::move(content));
}
