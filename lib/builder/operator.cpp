#include <llove/builder.hpp>
#include <llove/error.hpp>

llove::Operator<1>::Ptr llove::Builder::FindOperator(const std::string &operator_, const Field &operand, bool suffix)
{
    auto lowest_error = ~0u;
    Operator<1>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        const auto &function_type = function.Type;
        const auto parameter_count = function_type->GetParameterCount();

        if (function.Name != operator_)
            continue;

        if (suffix != function_type->HasVariadic())
            continue;

        auto error = 0u;

        if (const auto &function_self = function_type->GetSelf())
        {
            if (!function.Expose && function_self->GetType() != m_Class)
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

    if (candidate)
        return candidate;

    if (BIUnOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<1>>(BIUnOperatorCallees.at(operator_), suffix);

    return nullptr;
}

llove::Operator<2>::Ptr llove::Builder::FindOperator(
    const std::string &operator_,
    const Field &left,
    const Field &right)
{
    auto lowest_error = ~0u;
    Operator<2>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        const auto &function_type = function.Type;
        const auto parameter_count = function_type->GetParameterCount();

        if (function.Name != operator_)
            continue;

        if (function_type->HasVariadic())
            continue;

        auto error = 0u;

        if (const auto &function_self = function_type->GetSelf())
        {
            if (!function.Expose && function_self->GetType() != m_Class)
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

    if (candidate)
        return candidate;

    if (BIBiOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<2>>(BIBiOperatorCallees.at(operator_));

    return nullptr;
}
