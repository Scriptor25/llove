#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseScopeStatement()
{
    std::vector<StatementPtr> content;

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
        content.emplace_back(ParseStatement(false));
    Expect(TokenType_Other, "}");

    return std::make_unique<ScopeStatement>(std::move(content));
}
