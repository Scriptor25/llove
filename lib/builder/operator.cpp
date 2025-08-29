#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::Operator<1>::Ptr llove::Builder::FindOperator(const std::string &name, const Field &operand, bool suffix)
{
    auto lowest_error = ~0u;
    Operator<1>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        const auto &function_type = function.Type;
        const auto parameter_count = function_type->GetParameterCount();

        if (function.Name != name)
            continue;

        if (suffix != function_type->HasVariadic())
            continue;

        auto error = 0u;

        if (const auto &function_self = function_type->GetSelf())
        {
            if (!function.IsExposed && function_self->GetType() != m_Class)
                continue;
            if (parameter_count != 0)
                continue;
            if (Field::GetCastError(*this, *function_self, operand, error, true))
                continue;
        }
        else
        {
            if (parameter_count != 1)
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(0), operand, error, false))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(
            error != lowest_error,
            "ambiguous candidates '{}' and '{}' for operand '{}'",
            candidate,
            function,
            operand);

        lowest_error = error;
        candidate = std::make_unique<UDOperator<1>>(function);
    }

    if (operand.HasType() && operand.GetType()->IsClass())
    {
        const auto class_type = As<ClassType>(operand.GetType());
        for (const auto functions = class_type->GetFunctions(class_type, name);
             auto &[parent, function] : functions)
        {
            if (suffix != function.HasVariadic)
                continue;

            auto error = 0u;

            if (!function.IsExposed && parent != m_Class && m_Class->InheritsFrom(parent))
                continue;
            if (!function.Parameters.empty())
                continue;
            if (Field::GetCastError(*this, Field(function.IsMutable, true, parent), operand, error, true))
                continue;

            if (error > lowest_error)
                continue;

            Assert(
                error != lowest_error,
                "ambiguous candidates '{}' and '{}' for operand '{}'",
                candidate,
                function,
                operand);

            auto reference = GenFunction(
                {
                    .IsExport = function.IsExport,
                    .IsExposed = function.IsExposed,
                    .IsVirtual = function.IsVirtual,
                    .IsOverride = function.IsOverride,
                    .IsImplicit = function.IsImplicit,
                    .IsMutable = function.IsMutable,
                    .Class = parent,
                    .Name = function.Name,
                    .Variadic = { function.HasVariadic, {} },
                    .Result = function.Result,
                });

            lowest_error = error;
            candidate = std::make_unique<UDOperator<1>>(reference);
        }
    }

    if (candidate)
        return candidate;

    if (BIUnOperatorCallees.contains(name))
        return std::make_unique<BIOperator<1>>(BIUnOperatorCallees.at(name), suffix);

    return nullptr;
}

llove::Operator<2>::Ptr llove::Builder::FindOperator(
    const std::string &name,
    const Field &left,
    const Field &right)
{
    auto lowest_error = ~0u;
    Operator<2>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        const auto &function_type = function.Type;
        const auto parameter_count = function_type->GetParameterCount();

        if (function.Name != name)
            continue;

        if (function_type->HasVariadic())
            continue;

        auto error = 0u;

        if (const auto &function_self = function_type->GetSelf())
        {
            if (!function.IsExposed
                && function_self->GetType() != m_Class
                && !m_Class->InheritsFrom(function_self->GetType()))
                continue;
            if (parameter_count != 1)
                continue;
            if (Field::GetCastError(*this, *function_self, left, error, true))
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(0), right, error, false))
                continue;
        }
        else
        {
            if (parameter_count != 2)
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(0), left, error, false))
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(1), right, error, false))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(
            error != lowest_error,
            "ambiguous candidates '{}' and '{}' for operands '{}' and '{}'",
            candidate,
            function,
            left,
            right);

        lowest_error = error;
        candidate = std::make_unique<UDOperator<2>>(function);
    }

    if (left.HasType() && left.GetType()->IsClass())
    {
        const auto class_type = As<ClassType>(left.GetType());
        for (const auto functions = class_type->GetFunctions(class_type, name);
             auto &[parent, function] : functions)
        {
            const auto parameter_count = function.Parameters.size();

            if (function.HasVariadic)
                continue;

            auto error = 0u;

            if (!function.IsExposed
                && parent != m_Class
                && !m_Class->InheritsFrom(parent))
                continue;
            if (parameter_count != 1)
                continue;
            if (Field::GetCastError(*this, Field(function.IsMutable, true, parent), left, error, true))
                continue;
            if (Field::GetCastError(*this, function.Parameters.at(0), right, error, false))
                continue;

            if (error > lowest_error)
                continue;

            Assert(
                error != lowest_error,
                "ambiguous candidates '{}' and '{}' for operands '{}' and '{}'",
                candidate,
                function,
                left,
                right);

            auto reference = GenFunction(
                {
                    .IsExport = function.IsExport,
                    .IsExposed = function.IsExposed,
                    .IsVirtual = function.IsVirtual,
                    .IsOverride = function.IsOverride,
                    .IsImplicit = function.IsImplicit,
                    .IsMutable = function.IsMutable,
                    .Class = parent,
                    .Name = function.Name,
                    .Parameters = { { function.Parameters.at(0) } },
                    .Result = function.Result,
                });

            lowest_error = error;
            candidate = std::make_unique<UDOperator<2>>(reference);
        }
    }

    if (candidate)
        return candidate;

    if (BIBiOperatorCallees.contains(name))
        return std::make_unique<BIOperator<2>>(BIBiOperatorCallees.at(name));

    return nullptr;
}
