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

        ParseField(parameters.emplace_back(), false, true);

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    std::optional<Field> self;
    if (SkipIf(TokenType_Other, "["))
    {
        Field field;
        ParseField(field, false, true);
        Expect(TokenType_Other, "]");

        self = std::move(field);
    }

    Field result;
    if (SkipIf(TokenType_Operator, "=>"))
        ParseField(result, false, true);

    return m_Context.GetFunction(std::move(parameters), vararg, std::move(result), std::move(self));
}
