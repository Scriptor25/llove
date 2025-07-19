#include <llove/builder.hpp>
#include <llove/error.hpp>

bool llove::Builder::HasFunction(
    const std::vector<FunctionReference> &functions,
    const std::vector<Field> &arguments,
    const bool has_self,
    const Field &self) const
{
    for (const auto &function : functions)
    {
        const auto function_type = function.Type;
        if (function_type->HasSelf() != has_self)
            continue;

        if (has_self && !Field::IsCastable(*this, function_type->GetSelf(), self))
            continue;

        if (function_type->GetParameterCount() > arguments.size())
            continue;
        if (!function_type->IsVarArg() && function_type->GetParameterCount() < arguments.size())
            continue;

        unsigned i;
        for (i = 0; i < function_type->GetParameterCount(); ++i)
            if (!Field::IsCastable(*this, function_type->GetParameter(i), arguments.at(i)))
                break;
        if (i < function_type->GetParameterCount())
            continue;

        return true;
    }

    return false;
}

const llove::FunctionReference *llove::Builder::FindFunction(
    const std::vector<FunctionReference> &functions,
    const std::vector<Field> &arguments,
    const bool has_self,
    const Field &self) const
{
    auto lowest_error = ~0u;
    const FunctionReference *callee = nullptr;

    for (const auto &function : functions)
    {
        const auto function_type = function.Type;
        if (function_type->HasSelf() != has_self)
            continue;

        auto error = 0u;

        if (has_self && Field::GetCastError(*this, function_type->GetSelf(), self, error))
            continue;

        if (function_type->GetParameterCount() > arguments.size())
            continue;
        if (!function_type->IsVarArg() && function_type->GetParameterCount() < arguments.size())
            continue;

        if (function_type->GetParameterCount() != arguments.size())
            error += 2u;

        unsigned i;
        for (i = 0; i < function_type->GetParameterCount(); ++i)
            if (Field::GetCastError(*this, function_type->GetParameter(i), arguments.at(i), error))
                break;
        if (i < function_type->GetParameterCount())
            continue;

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        callee = &function;
    }

    return callee;
}

const llove::ClassFunctionReference *llove::Builder::FindFunction(
    const std::vector<ClassFunctionReference> &functions,
    const std::vector<Field> &arguments,
    const ClassType::Ptr &class_type,
    const Field &self) const
{
    auto lowest_error = ~0u;
    const ClassFunctionReference *candidate = nullptr;

    for (const auto &function : functions)
    {
        const Field class_
        {
            .Mutable = function.Mutable,
            .Reference = true,
            .Type = class_type,
        };

        if (!Field::IsAssignable(class_, self))
            continue;

        const auto parameter_count = function.Parameters.size();
        const auto argument_count = arguments.size();

        if (parameter_count > argument_count)
            continue;
        if (!function.VarArg && parameter_count < argument_count)
            continue;

        auto error = 0u;
        if (parameter_count != argument_count)
            error += 2u;

        unsigned i;
        for (i = 0; i < parameter_count; ++i)
            if (Field::GetCastError(*this, function.Parameters.at(i), arguments.at(i), error))
                break;
        if (i < parameter_count)
            continue;

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = &function;
    }

    return candidate;
}
