#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseTypeGlobal(bool export_)
{
    auto loc = Expect(TokenType_Symbol, "type").Loc;

    auto name = Expect(TokenType_Symbol).Value;
    Expect(TokenType_Operator, "=");
    auto type = ParseType();

    Expect(TokenType_Other, ";");

    return std::make_unique<TypeGlobal>(std::move(loc), export_, std::move(name), std::move(type));
}
