#include <llove/function.hpp>

std::ostream &llove::FunctionReference::Print(std::ostream &stream) const
{
    if (Expose)
        stream << "expose ";
    if (Implicit)
        stream << "implicit ";

    if (Type->HasSelf())
        stream << Type->GetSelf() << ':';

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

    stream << ')';

    if (Type->GetResult())
        stream << ": " << Type->GetResult();

    if (Delete)
        stream << " @delete";

    return stream;
}
