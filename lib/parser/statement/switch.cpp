#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::StatementPtr llove::Parser::ParseSwitchStatement()
{
    auto token = Expect(TokenType_Symbol, "switch");

    Expect(TokenType_Other, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Other, ")");

    std::vector<SwitchStatementCase> cases;
    auto has_default = false;

    Expect(TokenType_Other, "{");
    while (!At(TokenType_Other, "}"))
    {
        auto is_default = false;
        std::vector<ExpressionPtr> keys;

        Expect(TokenType_Other, "[");
        while (!At(TokenType_Other, "]"))
        {
            if (SkipIf(TokenType_Symbol, "default"))
            {
                Assert(!has_default, "multiple default cases specified");
                is_default = true;
                has_default = true;
            }
            else
            {
                keys.emplace_back(ParseExpression());
            }

            if (!At(TokenType_Other, "]"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, "]");

        StatementPtr content;
        if (SkipIf(TokenType_Operator, "->"))
        {
            content = ScopeStatement::Wrap(ParseStatement(false));
        }
        else
        {
            content = ParseScopeStatement();
        }

        cases.emplace_back(is_default, std::move(keys), std::move(content));
    }
    Expect(TokenType_Other, "}");

    return std::make_unique<SwitchStatement>(std::move(token.Loc), std::move(condition), std::move(cases));
}
