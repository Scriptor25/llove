#include <llove/forward.hpp>
#include <llove/tree.hpp>

std::ostream &llove::operator<<(std::ostream &stream, const Field &field)
{
    return field.Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const Parameter &parameter)
{
    return parameter.Print(stream);
}
