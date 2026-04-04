#include <llove/class.hpp>
#include <llove/tree.hpp>

std::ostream &llove::ClassMemberReference::Print(std::ostream &stream) const
{
    return Info.Print(stream << "let ", true, Name);
}

void llove::ClassMember::Reflect(Context &context, ClassMember &field) const
{
    field.Name = Name;
    Info.Reflect(context, field.Info);
}

std::ostream &llove::ClassMember::Print(std::ostream &stream) const
{
    return Info.Print(stream << "let ", true, Name) << ';';
}

std::ostream &llove::ClassFunctionReference::Print(std::ostream &stream) const
{
    stream
            << (IsPublic ? "public " : "")
            << (IsImplicit ? "implicit " : "")
            << (IsMutable ? "mut " : "")
            << Name
            << '(';
    for (auto i = Parameters.begin(); i != Parameters.end(); ++i)
    {
        if (i != Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (IsVariadic)
    {
        if (!Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    return stream << "): " << Result;
}

void llove::ClassFunction::Reflect(Context &context, ClassFunction &function) const
{
    function.Loc = Loc;
    function.IsPublic = IsPublic;
    function.IsVirtual = IsVirtual;
    function.IsOverride = IsOverride;
    function.IsImplicit = IsImplicit;
    function.IsMutable = IsMutable;
    function.Name = Name;
    function.Variadic = Variadic;

    function.Parameters.resize(Parameters.size());
    for (unsigned i = 0; i < Parameters.size(); ++i)
    {
        function.Parameters[i].Name = Parameters[i].Name;
        Parameters[i].Info.Reflect(context, function.Parameters[i].Info);
    }

    Result.Reflect(context, function.Result);

    if (Content)
        Content->Reflect(context, function.Content);
}

std::ostream &llove::ClassFunction::Print(std::ostream &stream) const
{
    stream
            << (IsPublic ? "public " : "")
            << (IsVirtual ? "virtual " : "")
            << (IsOverride ? "override " : "")
            << (IsImplicit ? "implicit " : "")
            << (IsMutable ? "mut " : "")
            << Name
            << '(';
    for (auto i = Parameters.begin(); i != Parameters.end(); ++i)
    {
        if (i != Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (Variadic.Is)
    {
        if (!Parameters.empty())
            stream << ", ";
        stream << "...";
        if (!Variadic.Name.empty())
            stream << Variadic.Name;
    }
    stream << "): " << Result;
    if (!Content)
        return stream << ';';
    return stream << ' ' << Content;
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassMember &field)
{
    return field.Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassFunction &function)
{
    return function.Print(stream);
}
