#include <llove/function.hpp>
#include <llove/tree.hpp>

std::ostream &llove::FunctionReference::Print(std::ostream &stream) const
{
    if (IsPublic)
        stream << "public ";
    if (IsImplicit)
        stream << "implicit ";

    if (const auto self = Type->GetSelf())
        stream << *self << ':';

    stream << Name << '(';

    for (unsigned i = 0; i < Type->GetParameterCount(); ++i)
    {
        if (i)
            stream << ", ";
        stream << Type->GetParameter(i);
    }
    if (Type->IsVariadic())
    {
        if (Type->GetParameterCount())
            stream << ", ";
        stream << "...";
    }

    return stream << "): " << Type->GetResult();
}

void llove::Initializer::Reflect(Context &context, Initializer &initializer) const
{
    initializer.Name = Name;
    if (Value)
        Value->Reflect(context, initializer.Value);
    for (auto &argument : Arguments)
        argument->Reflect(context, initializer.Arguments.emplace_back());
}
