#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::Location llove::Parser::ParseParameterList(
    std::vector<Parameter> &parameters,
    Variadic &variadic)
{
    return ParseList<Parameter, Variadic>(
        parameters,
        variadic,
        [&](Parameter &element)
        {
            ParseParameter(element);
        },
        [&](Variadic &ellipsis)
        {
            ellipsis.Is = true;
            if (At(TokenType_Symbol))
                ellipsis.Name = std::move(Skip().Value);
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
