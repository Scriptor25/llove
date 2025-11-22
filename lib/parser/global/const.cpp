#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseConstGlobal(const bool export_)
{
    auto token = Expect(TokenType_Symbol, "const");

    auto name = Expect(TokenType_Symbol).Value;

    TypePtr type;
    if (SkipIf(TokenType_Operator, ":"))
        type = ParseType();

    Expect(TokenType_Operator, "=");
    auto value = ParseExpression();

    Expect(TokenType_Other, ";");

    return std::make_unique<ConstGlobal>(std::move(token.Loc), export_, std::move(name), std::move(type), std::move(value));
}
