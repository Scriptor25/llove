#include <llove/parser.hpp>

std::pair<bool, std::string> llove::Parser::ParseParameterList(
    const std::string &begin,
    std::vector<Parameter> &parameters,
    const std::string &end)
{
    auto is_variadic = false;
    std::string variadic_name;

    Expect(TokenType_Other, begin);
    while (!At(TokenType_Other, end))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            is_variadic = true;
            if (At(TokenType_Symbol))
                variadic_name = Skip().Value;
            break;
        }

        ParseParameter(parameters.emplace_back());

        if (!At(TokenType_Other, end))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, end);

    return { is_variadic, variadic_name };
}
