#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseTypeGlobal(
    bool /* is_template */,
    bool is_export)
{
    auto loc = Expect(TokenType_Symbol, "type").Loc;

    auto name = Expect(TokenType_Symbol).Value;
    Expect(TokenType_Operator, "=");
    auto type = ParseType();

    Expect(TokenType_Other, ";");

    return std::make_unique<TypeGlobal>(std::move(loc), is_export, std::move(name), std::move(type));
}
