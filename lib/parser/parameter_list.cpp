#include <llove/parser.hpp>

bool llove::Parser::ParseParameterList(
    const std::string &begin,
    std::vector<Parameter> &parameters,
    const std::string &end)
{
    auto vararg = false;

    Expect(TokenType_Other, begin);
    while (!At(TokenType_Other, end))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            vararg = true;
            break;
        }

        ParseParameter(parameters.emplace_back());

        if (!At(TokenType_Other, end))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, end);

    return vararg;
}
