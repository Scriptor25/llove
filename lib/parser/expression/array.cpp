#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseArrayExpression()
{
    auto token = Expect(TokenType_Other, "[");

    std::vector<ExpressionPtr> values;
    while (!At(TokenType_Other, "]"))
    {
        values.emplace_back(ParseExpression());

        if (!At(TokenType_Other, "]"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, "]");

    TypePtr type;
    if (SkipIf(TokenType_Other, ":"))
        type = m_Types.GetArray(ParseType(), values.size());

    return std::make_unique<ArrayExpression>(std::move(token.Loc), std::move(values), std::move(type));
}
