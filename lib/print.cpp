#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <ostream>

std::ostream& llove::operator<<(
    std::ostream& stream,
    const Field& field)
{
    return field.Print(stream);
}

std::ostream& llove::operator<<(
    std::ostream& stream,
    const Parameter& parameter)
{
    return parameter.Print(stream);
}

std::ostream& llove::operator<<(
    std::ostream& stream,
    const GlobalPtr& ptr)
{
    return ptr->Print(stream);
}

std::ostream& llove::operator<<(
    std::ostream& stream,
    const StatementPtr& ptr)
{
    if (dynamic_cast<Expression*>(ptr.get()))
        return ptr->Print(stream) << ';';

    return ptr->Print(stream);
}

std::ostream& llove::operator<<(
    std::ostream& stream,
    const ExpressionPtr& ptr)
{
    return ptr->Print(stream);
}
