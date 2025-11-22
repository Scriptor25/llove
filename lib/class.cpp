#include <llove/class.hpp>
#include <llove/tree.hpp>

std::ostream& llove::ClassMemberReference::Print(std::ostream& stream) const
{
    return Info.Print(stream << "let ", true, Name);
}

void llove::ClassMember::Reflect(
    Context& context,
    ClassMember& field) const
{
    field.Name = Name;

    Info.Reflect(context, field.Info);

    if (Value)
        Value->Reflect(context, field.Value);

    field.Arguments.resize(Arguments.size());
    for (unsigned i = 0; i < Arguments.size(); ++i)
        Arguments.at(i)->Reflect(context, field.Arguments.at(i));
}

std::ostream& llove::ClassMember::Print(std::ostream& stream) const
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

std::ostream& llove::ClassFunctionReference::Print(std::ostream& stream) const
{
    stream << (IsExposed ? "expose " : "") << (IsImplicit ? "implicit " : "") << (IsMutable ? "mut " : "") << Name << '(';
    for (auto i = Parameters.begin(); i != Parameters.end(); ++i)
    {
        if (i != Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (HasVariadic)
    {
        if (!Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    return stream << "): " << Result;
}

void llove::ClassFunction::Reflect(
    Context& context,
    ClassFunction& function) const
{
    function.Loc = Loc;
    function.IsExposed = IsExposed;
    function.IsVirtual = IsVirtual;
    function.IsOverride = IsOverride;
    function.IsImplicit = IsImplicit;
    function.IsMutable = IsMutable;
    function.Name = Name;
    function.Variadic = Variadic;

    function.Parameters.resize(Parameters.size());
    for (unsigned i = 0; i < Parameters.size(); ++i)
    {
        function.Parameters.at(i).Name = Parameters.at(i).Name;
        Parameters.at(i).Info.Reflect(context, function.Parameters.at(i).Info);
    }

    Result.Reflect(context, function.Result);

    if (Content)
        Content->Reflect(context, function.Content);
}

std::ostream& llove::ClassFunction::Print(std::ostream& stream) const
{
    stream << (IsExposed ? "expose " : "") << (IsVirtual ? "virtual " : "") << (IsOverride ? "override " : "") << (IsImplicit ? "implicit " : "") << (IsMutable ? "mut " : "") << Name << '(';
    for (auto i = Parameters.begin(); i != Parameters.end(); ++i)
    {
        if (i != Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (Variadic.first)
    {
        if (!Parameters.empty())
            stream << ", ";
        stream << "...";
        if (!Variadic.second.empty())
            stream << Variadic.second;
    }
    stream << "): " << Result;
    if (!Content)
        return stream << ';';
    return stream << ' ' << Content;
}

std::ostream& llove::operator<<(
    std::ostream& stream,
    const ClassMember& field)
{
    return field.Print(stream);
}

std::ostream& llove::operator<<(
    std::ostream& stream,
    const ClassFunction& function)
{
    return function.Print(stream);
}
