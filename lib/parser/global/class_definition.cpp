#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassDefinitionGlobal(Location loc)
{
    auto class_name = Expect(TokenType_Symbol).Value;
    auto class_type = m_Context.GetClass(std::move(class_name));

    auto mutable_ = SkipIf(TokenType_Symbol, "mut");
    auto name = At(TokenType_Operator) ? Skip().Value : Expect(TokenType_Symbol).Value;

    std::vector<Parameter> parameters;
    auto vararg = ParseParameterList("(", parameters, ")");

    Field result;
    if (SkipIf(TokenType_Other, ":"))
        ParseField(result, false, true);
    else
        result.Type = m_Context.GetVoid();

    auto content = ParseScopeStatement();

    return std::make_unique<ClassDefinitionGlobal>(
        std::move(loc),
        std::move(class_type),
        mutable_,
        std::move(name),
        std::move(parameters),
        vararg,
        std::move(result),
        std::move(content));
}
