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

        auto function_type = function.Type;
        if (suffix != function_type->IsVarArg())
            continue;

        auto error = 0u;

        if (function_type->HasSelf())
        {
            if (function_type->GetParameterCount() != 0)
                continue;
            if (Field::GetCastError(*this, function_type->GetSelf(), operand, error))
                continue;
        }
        else
        {
            if (function_type->GetParameterCount() != 1)
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(0), operand, error))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<1>>(function_type, function.Callee);
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

        auto function_type = function.Type;
        if (function_type->IsVarArg())
            continue;

        auto error = 0u;

        if (function_type->HasSelf())
        {
            if (function_type->GetParameterCount() != 1)
                continue;
            if (Field::GetCastError(*this, function_type->GetSelf(), left, error))
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(0), right, error))
                continue;
        }
        else
        {
            if (function_type->GetParameterCount() != 2)
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(0), left, error))
                continue;
            if (Field::GetCastError(*this, function_type->GetParameter(1), right, error))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<2>>(function_type, function.Callee);
    }

    if (candidate)
        return candidate;

    if (BIBiOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<2>>(BIBiOperatorCallees.at(operator_));

    return nullptr;
}
