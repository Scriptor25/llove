#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseOperandExpression()
{
    auto expression = ParsePrimaryExpression();

    while (true)
    {
        if (SkipIf(TokenType_Otr, "("))
        {
            std::vector<ExpressionPtr> arguments;

            while (!At(TokenType_Otr, ")"))
            {
                arguments.emplace_back(ParseExpression());

                if (!At(TokenType_Otr, ")"))
                    Expect(TokenType_Otr, ",");
            }
            Expect(TokenType_Otr, ")");

            expression = std::make_unique<CallExpression>(std::move(expression), std::move(arguments));
            continue;
        }

        if (SkipIf(TokenType_Opr, "."))
        {
            auto member = Expect(TokenType_Sym).Value;

            expression = std::make_unique<MemberExpression>(std::move(expression), std::move(member));
            continue;
        }

        if (SkipIf(TokenType_Opr, ".."))
        {
            auto end = ParsePrimaryExpression();

            expression = std::make_unique<RangeExpression>(std::move(expression), std::move(end));
            continue;
        }

        if (SkipIf(TokenType_Otr, "["))
        {
            auto index = ParseExpression();
            Expect(TokenType_Otr, "]");

            expression = std::make_unique<SubscriptExpression>(std::move(expression), std::move(index));
            continue;
        }

        if (At(TokenType_Opr, "++", "--"))
        {
            auto operator_ = Skip().Value;

            expression = std::make_unique<UnaryExpression>(std::move(operator_), std::move(expression), true);
            continue;
        }

        return expression;
    }
}
