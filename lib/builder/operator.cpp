#include <llove/builder.hpp>
#include <llove/error.hpp>

llove::Operator<1>::Ptr llove::Builder::FindOperator(const std::string &operator_, const Field &operand, bool suffix)
{
    auto lowest_error = ~0u;
    Operator<1>::Ptr candidate;

    for (auto &[
             expose,
             implicit,
             name,
             type,
             callee
         ] : m_Functions)
    {
        if (name != operator_)
            continue;

        if (suffix != type->IsVarArg())
            continue;

        auto error = 0u;

        if (type->HasSelf())
        {
            if (!expose && type->GetSelf().Type != m_Class)
                continue;
            if (type->GetParameterCount() != 0)
                continue;
            if (Field::GetCastError(*this, type->GetSelf(), operand, error, true))
                continue;
        }
        else
        {
            if (type->GetParameterCount() != 1)
                continue;
            if (Field::GetCastError(*this, type->GetParameter(0), operand, error, false))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<1>>(type, callee);
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

    for (auto &[
             expose,
             implicit,
             name,
             type,
             callee
         ] : m_Functions)
    {
        if (name != operator_)
            continue;

        if (type->IsVarArg())
            continue;

        auto error = 0u;

        if (type->HasSelf())
        {
            if (!expose && type->GetSelf().Type != m_Class)
                continue;
            if (type->GetParameterCount() != 1)
                continue;
            if (Field::GetCastError(*this, type->GetSelf(), left, error, true))
                continue;
            if (Field::GetCastError(*this, type->GetParameter(0), right, error, false))
                continue;
        }
        else
        {
            if (type->GetParameterCount() != 2)
                continue;
            if (Field::GetCastError(*this, type->GetParameter(0), left, error, false))
                continue;
            if (Field::GetCastError(*this, type->GetParameter(1), right, error, false))
                continue;
        }

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = std::make_unique<UDOperator<2>>(type, callee);
    }

    if (candidate)
        return candidate;

    if (BIBiOperatorCallees.contains(operator_))
        return std::make_unique<BIOperator<2>>(BIBiOperatorCallees.at(operator_));

    return nullptr;
}
