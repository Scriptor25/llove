#include <llove/parser.hpp>
#include <llove/tree.hpp>

using namespace std::placeholders;

llove::Location llove::Parser::ParseParameterList(
    std::vector<Parameter> &parameters,
    std::pair<bool, std::string> &variadic)
{
    return ParseList<Parameter, std::pair<bool, std::string>>(
        parameters,
        variadic,
        std::bind(&Parser::ParseParameter, this, _1),
        [&](std::pair<bool, std::string> &ellipsis)
        {
            ellipsis.first = true;
            if (At(TokenType_Symbol))
                ellipsis.second = std::move(Skip().Value);
        },
        TokenType_Other,
        "(",
        TokenType_Other,
        ")",
        TokenType_Operator,
        "...");
}

llove::Location llove::Parser::ParseTemplateParameterList(
    std::vector<std::pair<std::string, TemplateType::Ptr>> &parameters)
{
    return ParseList<TemplateParameter>(
        parameters,
        [&](TemplateParameter &element)
        {
            auto parameter_name = Expect(TokenType_Symbol).Value;
            element = { parameter_name, std::make_shared<TemplateType>(parameter_name) };
        },
        TokenType_Operator,
        "<",
        TokenType_Operator,
        ">");
}
