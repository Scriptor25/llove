#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseFunctionType()
{
    Expect(TokenType_Other, "(");

    std::vector<Field> parameters;
    auto vararg = false;

    while (!At(TokenType_Other, ")"))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            vararg = true;
            break;
        }

        ParseField(parameters.emplace_back(), false, false);

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    std::optional<Field> self;
    if (SkipIf(TokenType_Other, "["))
    {
        ParseField(*self, false, false);
        Expect(TokenType_Other, "]");
    }

    Field result;
    if (SkipIf(TokenType_Operator, "=>"))
        ParseField(result, false, false);

    if (self)
        return m_Types.GetFunction(std::move(parameters), vararg, std::move(result), std::move(*self));

    return m_Types.GetFunction(std::move(parameters), vararg, std::move(result));
}
