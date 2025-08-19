#include <llove/parser.hpp>

std::pair<bool, std::string> llove::Parser::ParseParameterList(
    const std::string &begin,
    std::vector<Parameter> &parameters,
    const std::string &end)
{
    auto vararg = false;
    std::string vararg_name;

    Expect(TokenType_Other, begin);
    while (!At(TokenType_Other, end))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            vararg = true;
            if (At(TokenType_Symbol))
                vararg_name = Skip().Value;
            break;
        }

        ParseParameter(parameters.emplace_back());

        if (!At(TokenType_Other, end))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, end);

    return { vararg, vararg_name };
}
