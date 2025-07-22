#include <llove/class.hpp>

std::ostream &llove::operator<<(std::ostream &stream, const ClassField &field)
{
    field.Info.Print(stream << "let ", true, field.Name);
    if (field.Value)
        stream << " = " << field.Value;
    else if (!field.Arguments.empty())
    {
        stream << '(';
        for (auto i = field.Arguments.begin(); i != field.Arguments.end(); ++i)
        {
            if (i != field.Arguments.begin())
                stream << ", ";
            stream << *i;
        }
        stream << ')';
    }
    return stream << ';';
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
