#include <llove/class.hpp>
#include <llove/tree.hpp>

void llove::ClassField::Reflect(Context &types, ClassField &field) const
{
    field.Name = Name;

    Info.Reflect(types, field.Info);

    Value->Reflect(types, field.Value);

    field.Arguments.resize(Arguments.size());
    for (unsigned i = 0; i < Arguments.size(); ++i)
        Arguments.at(i)->Reflect(types, field.Arguments.at(i));
}

void llove::ClassFunction::Reflect(Context &types, ClassFunction &function) const
{
    function.Expose = Expose;
    function.Mutable = Mutable;
    function.Name = Name;
    function.VarArg = VarArg;

    function.Parameters.resize(Parameters.size());
    for (unsigned i = 0; i < Parameters.size(); ++i)
    {
        function.Parameters.at(i).Name = Parameters.at(i).Name;
        Parameters.at(i).Info.Reflect(types, function.Parameters.at(i).Info);
    }

    Result.Reflect(types, function.Result);

    Content->Reflect(types, function.Content);
}

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
