#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseOperandExpression()
{
    auto expression = ParsePrimaryExpression();

    while (true)
    {
        auto loc = m_Token.Loc;

        if (SkipIf(TokenType_Other, "("))
        {
            std::vector<ExpressionPtr> arguments;

            while (!At(TokenType_Other, ")"))
            {
                arguments.emplace_back(ParseExpression());

                if (!At(TokenType_Other, ")"))
                    Expect(TokenType_Other, ",");
            }
            Expect(TokenType_Other, ")");

            expression = std::make_unique<CallExpression>(std::move(loc), std::move(expression), std::move(arguments));
            continue;
        }

        if (SkipIf(TokenType_Operator, "."))
        {
            auto member = Expect(TokenType_Symbol).Value;

            expression = std::make_unique<MemberExpression>(std::move(loc), std::move(expression), std::move(member));
            continue;
        }

        if (SkipIf(TokenType_Operator, ".."))
        {
            auto end = ParsePrimaryExpression();

            expression = std::make_unique<RangeExpression>(std::move(loc), std::move(expression), std::move(end));
            continue;
        }

        if (SkipIf(TokenType_Other, "["))
        {
            auto index = ParseExpression();
            Expect(TokenType_Other, "]");

            expression = std::make_unique<SubscriptExpression>(std::move(loc), std::move(expression), std::move(index));
            continue;
        }

        if (At(TokenType_Operator, "++", "--"))
        {
            auto operator_ = Skip().Value;

            expression = std::make_unique<UnaryExpression>(
                std::move(loc),
                std::move(operator_),
                std::move(expression),
                true);
            continue;
        }

        return expression;
    }
}
