#include <llove/builder.hpp>
#include <llove/error.hpp>

llove::Operator<1>::Ptr llove::Builder::FindOperator(const std::string &operator_, const Field &operand, bool suffix)
{
    auto lowest_error = ~0u;
    Operator<1>::Ptr candidate;

    for (auto &function : m_Functions)
    {
        if (function.Name != operator_)
            continue;

        if (suffix != function.Type->IsVarArg())
            continue;

        auto error = 0u;

        if (function.Type->HasSelf())
        {
            if (!function.Expose && function.Type->GetSelf().Type != m_Class)
                continue;
            if (function.Type->GetParameterCount() != 0)
                continue;
            if (Field::GetCastError(*this, function.Type->GetSelf(), operand, error, true))
                continue;
        }
        else
        {
            if (function.Type->GetParameterCount() != 1)
                continue;
            if (Field::GetCastError(*this, function.Type->GetParameter(0), operand, error, false))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

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
        if (function.Name != operator_)
            continue;

        if (function.Type->IsVarArg())
            continue;

        auto error = 0u;

        if (function.Type->HasSelf())
        {
            if (!function.Expose && function.Type->GetSelf().Type != m_Class)
                continue;
            if (function.Type->GetParameterCount() != 1)
                continue;
            if (Field::GetCastError(*this, function.Type->GetSelf(), left, error, true))
                continue;
            if (Field::GetCastError(*this, function.Type->GetParameter(0), right, error, false))
                continue;
        }
        else
        {
            if (function.Type->GetParameterCount() != 2)
                continue;
            if (Field::GetCastError(*this, function.Type->GetParameter(0), left, error, false))
                continue;
            if (Field::GetCastError(*this, function.Type->GetParameter(1), right, error, false))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<2>>(function);
    }

    if (candidate)
        return candidate;

    if (BIBiOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<2>>(BIBiOperatorCallees.at(operator_));

    return nullptr;
}
