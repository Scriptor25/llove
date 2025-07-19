#include <llove/parameter.hpp>

std::ostream &llove::Parameter::Print(std::ostream &stream) const
{
    return Info.Print(stream, true, Name);
}
