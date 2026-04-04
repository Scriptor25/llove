#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseSwitchExpression()
{
    auto token = Expect(TokenType_Symbol, "switch");

    Expect(TokenType_Other, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Other, ")");

    std::vector<SwitchExpressionCase> cases;
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
                keys.push_back(ParseExpression());
            }

            if (!At(TokenType_Other, "]"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, "]");

        Expect(TokenType_Operator, "->");
        auto value = ParseExpression();
        Expect(TokenType_Other, ";");

        cases.emplace_back(is_default, std::move(keys), std::move(value));
    }
    Expect(TokenType_Other, "}");

    Assert(has_default, token.Loc, "switch expression requires default case");

    return std::make_unique<SwitchExpression>(std::move(token.Loc), std::move(condition), std::move(cases));
}
