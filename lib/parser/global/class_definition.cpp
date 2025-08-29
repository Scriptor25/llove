#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassDefinitionGlobal(Location loc)
{
    static const std::set<std::string_view> no_result
    {
        "create",
        "delete",
    };

    auto class_name = Expect(TokenType_Symbol).Value;
    auto class_type = m_Context.GetClass(std::move(class_name));

    auto is_mutable = SkipIf(TokenType_Symbol, "mut");
    auto name = At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    std::vector<Parameter> parameters;
    std::pair<bool, std::string> variadic;
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
            [this](auto &element)
            {
                ParseInitializer(element);
            },
            TokenType_Other,
            "[",
            TokenType_Other,
            "]");

    auto content = ParseScopeStatement();

    return std::make_unique<ClassDefinitionGlobal>(
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
