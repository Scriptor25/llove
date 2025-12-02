#include <llove/function.hpp>
#include <llove/tree.hpp>
#include <utility>

std::ostream& llove::FunctionReference::Print(std::ostream& stream) const
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

llove::Initializer::Initializer(
    std::string name,
    ExpressionPtr value,
    std::vector<ExpressionPtr> arguments)
    : Name(std::move(name)),
      Value(std::move(value)),
      Arguments(std::move(arguments))
{
}

void llove::Initializer::Reflect(
    Context& context,
    Initializer& initializer) const
{
    initializer.Name = Name;
    if (Value)
        Value->Reflect(context, initializer.Value);
    for (auto& argument : Arguments)
        argument->Reflect(context, initializer.Arguments.emplace_back());
}
