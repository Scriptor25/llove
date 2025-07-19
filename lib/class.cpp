#include <llove/class.hpp>

std::ostream &llove::operator<<(std::ostream &stream, const ClassFieldReference &field)
{
    return field.Info.Print(stream << "let ", true, field.Name) << ';';
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassFunction &function)
{
    stream
            << (function.Expose ? "expose " : "")
            << (function.Mutable ? "mut " : "")
            << function.Name
            << '(';
    for (auto i = function.Parameters.begin(); i != function.Parameters.end(); ++i)
    {
        if (i != function.Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (function.VarArg)
    {
        if (!function.Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    stream << ')';
    if (function.Result)
        stream << ": " << function.Result;
    if (!function.Content)
        return stream << ';';
    return stream << ' ' << function.Content;
}
