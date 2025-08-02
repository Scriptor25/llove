#include <llove/function.hpp>

std::ostream &llove::FunctionReference::Print(std::ostream &stream) const
{
    if (Expose)
        stream << "expose ";
    if (Implicit)
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
    if (Type->IsVarArg())
    {
        if (Type->GetParameterCount())
            stream << ", ";
        stream << "...";
    }

    return stream << "): " << Type->GetResult();
}
