#include <llove/class.hpp>
#include <llove/tree.hpp>

std::ostream &llove::ClassFieldReference::Print(std::ostream &stream) const
{
    return Info.Print(stream << "let ", true, Name);
}

std::ostream &llove::ClassFunctionReference::Print(std::ostream &stream) const
{
    stream
            << (Expose ? "expose " : "")
            << (Implicit ? "implicit " : "")
            << (Mutable ? "mut " : "")
            << Name
            << '(';
    for (auto i = Parameters.begin(); i != Parameters.end(); ++i)
    {
        if (i != Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (VarArg)
    {
        if (!Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    stream << ')';
    if (Result)
        stream << ": " << Result;
    return stream;
}

void llove::ClassField::Reflect(Context &types, ClassField &field) const
{
    field.Name = Name;

    Info.Reflect(types, field.Info);

    if (Value)
        Value->Reflect(types, field.Value);

    field.Arguments.resize(Arguments.size());
    for (unsigned i = 0; i < Arguments.size(); ++i)
        Arguments.at(i)->Reflect(types, field.Arguments.at(i));
}

std::ostream &llove::ClassField::Print(std::ostream &stream) const
{
    Info.Print(stream << "let ", true, Name);
    if (Value)
        stream << " = " << Value;
    else if (!Arguments.empty())
    {
        stream << '(';
        for (auto i = Arguments.begin(); i != Arguments.end(); ++i)
        {
            if (i != Arguments.begin())
                stream << ", ";
            stream << *i;
        }
        stream << ')';
    }
    return stream << ';';
}

void llove::ClassFunction::Reflect(Context &types, ClassFunction &function) const
{
    function.Expose = Expose;
    function.Implicit = Implicit;
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

    if (Content)
        Content->Reflect(types, function.Content);
}

std::ostream &llove::ClassFunction::Print(std::ostream &stream) const
{
    stream
            << (Expose ? "expose " : "")
            << (Implicit ? "implicit " : "")
            << (Mutable ? "mut " : "")
            << Name
            << '(';
    for (auto i = Parameters.begin(); i != Parameters.end(); ++i)
    {
        if (i != Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (VarArg)
    {
        if (!Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    stream << ')';
    if (Result)
        stream << ": " << Result;
    if (!Content)
        return stream << ';';
    return stream << ' ' << Content;
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassField &field)
{
    return field.Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassFunction &function)
{
    return function.Print(stream);
}
