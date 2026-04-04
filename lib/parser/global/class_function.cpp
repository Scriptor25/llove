#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassFunctionGlobal(Location loc)
{
    static const std::set<std::string_view> no_result
    {
        "create",
        "delete",
    };

    auto class_name = Expect(TokenType_Symbol).Value;
    auto class_type = m_Context.GetClass(std::move(class_name));

    auto is_mutable = SkipIf(TokenType_Symbol, "mut");

    std::string name;
    if (At(TokenType_Symbol) || At(TokenType_Operator))
        name = Skip().Value;
    else if (SkipIf(TokenType_Other, "("))
    {
        Expect(TokenType_Other, ")");
        name = "()";
    }
    else if (SkipIf(TokenType_Other, "["))
    {
        Expect(TokenType_Other, "]");
        name = "[]";
    }

    std::vector<Parameter> parameters;
    Variadic variadic;
    ParseParameterList(parameters, variadic);

    Field result;
    if (!no_result.contains(name) && SkipIf(TokenType_Operator, ":"))
        ParseField(result, false, true);
    else
        result.SetType(m_Context.GetVoid());

    std::vector<Initializer> initializers;
    if (name == "create" && At(TokenType_Other, "["))
        ParseList<Initializer>(
            initializers,
            [&](Initializer &element)
            {
                return ParseInitializer(element);
            },
            TokenType_Other,
            "[",
            TokenType_Other,
            "]");

    StatementPtr content;
    if (At(TokenType_Operator, "->"))
    {
        auto ret_loc = Skip().Loc;

        auto expression = ParseExpression();
        Expect(TokenType_Other, ";");

        if (result.GetType()->IsVoid())
            content = std::move(expression);
        else
            content = std::make_unique<RetStatement>(std::move(ret_loc), std::move(expression));
    }
    else
        content = ParseScopeStatement();

    return std::make_unique<ClassFunctionGlobal>(
        std::move(loc),
        std::move(class_type),
        is_mutable,
        std::move(name),
        std::move(parameters),
        variadic,
        std::move(result),
        std::move(initializers),
        std::move(content));
}
