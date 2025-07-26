#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseScopeStatement()
{
    std::vector<StatementPtr> content;

    Expect(TokenType_Otr, "{");
    while (!At(TokenType_Otr, "}"))
        content.emplace_back(ParseStatement(false));
    Expect(TokenType_Otr, "}");

    return std::make_unique<ScopeStatement>(std::move(content));
}
