#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseScopeStatement()
{
    std::vector<StatementPtr> content;

    auto loc = Expect(TokenType_Other, "{").Loc;
    while (!At(TokenType_Other, "}"))
        content.push_back(ParseStatement(false));
    Expect(TokenType_Other, "}");

    return std::make_unique<ScopeStatement>(std::move(loc), std::move(content));
}
