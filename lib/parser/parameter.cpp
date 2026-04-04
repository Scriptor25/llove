#include <llove/parser.hpp>

void llove::Parser::ParseParameter(Parameter &parameter)
{
    parameter.Name = ParseField(parameter.Info, true, true);
}
