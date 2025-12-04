#include <llove/forward.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseLetGlobal(bool is_export)
{
    auto loc = Expect(TokenType_Symbol, "let").Loc;

    auto name = Expect(TokenType_Symbol).Value;

    Expect(TokenType_Operator, ":");

    auto type = ParseType();

    Expect(TokenType_Other, ";");

    return std::make_unique<LetGlobal>(std::move(loc), is_export, std::move(name), std::move(type));
}
