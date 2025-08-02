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
    return stream << "): " << Result;
}

void llove::ClassField::Reflect(Builder &builder, ClassField &field) const
{
    field.Name = Name;

    Info.Reflect(builder, field.Info);

    if (Value)
        Value->Reflect(builder, field.Value);

    field.Arguments.resize(Arguments.size());
    for (unsigned i = 0; i < Arguments.size(); ++i)
        Arguments.at(i)->Reflect(builder, field.Arguments.at(i));
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

void llove::ClassFunction::Reflect(Builder &builder, ClassFunction &function) const
{
    function.Loc = Loc;
    function.Expose = Expose;
    function.Implicit = Implicit;
    function.Mutable = Mutable;
    function.Name = Name;
    function.VarArg = VarArg;

    function.Parameters.resize(Parameters.size());
    for (unsigned i = 0; i < Parameters.size(); ++i)
    {
        function.Parameters.at(i).Name = Parameters.at(i).Name;
        Parameters.at(i).Info.Reflect(builder, function.Parameters.at(i).Info);
    }

    Result.Reflect(builder, function.Result);

    if (Content)
        Content->Reflect(builder, function.Content);
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
    stream << "): " << Result;
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
