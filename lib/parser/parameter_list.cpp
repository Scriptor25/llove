#include <llove/parser.hpp>

std::pair<bool, std::string> llove::Parser::ParseParameterList(std::vector<Parameter> &parameters)
{
    auto is_variadic = false;
    std::string variadic_name;

    Expect(TokenType_Other, "(");
    while (!At(TokenType_Other, ")"))
    {
        if (SkipIf(TokenType_Operator, "..."))
        {
            is_variadic = true;
            if (At(TokenType_Symbol))
                variadic_name = Skip().Value;
            break;
        }

        ParseParameter(parameters.emplace_back());

        if (!At(TokenType_Other, ")"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, ")");

    return { is_variadic, variadic_name };
}

void llove::Parser::ParseTemplateParameterList(std::vector<std::pair<std::string, TemplateType::Ptr>> &parameters)
{
    Expect(TokenType_Operator, "<");
    while (!At(TokenType_Operator, ">"))
    {
        auto parameter_name = Expect(TokenType_Symbol).Value;
        parameters.emplace_back(parameter_name, std::make_shared<TemplateType>(parameter_name));

        if (!At(TokenType_Operator, ">"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Operator, ">");
}
