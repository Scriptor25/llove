#include <llove/function.hpp>

std::ostream &llove::FunctionReference::Print(std::ostream &stream) const
{
    if (IsExposed)
        stream << "expose ";
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
    if (Type->HasVariadic())
    {
        if (Type->GetParameterCount())
            stream << ", ";
        stream << "...";
    }

    return stream << "): " << Type->GetResult();
}
