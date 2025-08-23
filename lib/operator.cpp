#include <utility>
#include <llove/builder.hpp>
#include <llove/operator.hpp>

llove::BIOperator<1>::BIOperator(CalleeType callee, const bool suffix)
    : m_Callee(std::move(callee)),
      m_Suffix(suffix)
{
}

llove::ValuePtr llove::BIOperator<1>::operator()(Builder &builder, ValuePtr operand) const
{
    return m_Callee(builder, std::move(operand), m_Suffix);
}

std::ostream &llove::BIOperator<1>::Print(std::ostream &stream) const
{
    return stream << "builtin";
}

llove::BIOperator<2>::BIOperator(CalleeType callee)
    : m_Callee(std::move(callee))
{
}

llove::ValuePtr llove::BIOperator<2>::operator()(
    Builder &builder,
    ValuePtr left,
    ValuePtr right) const
{
    return m_Callee(builder, std::move(left), std::move(right));
}

std::ostream &llove::BIOperator<2>::Print(std::ostream &stream) const
{
    return stream << "builtin";
}

llove::UDOperator<1>::UDOperator(const FunctionReference &reference)
    : m_Reference(std::move(reference))
{
}

llove::ValuePtr llove::UDOperator<1>::operator()(Builder &builder, ValuePtr operand) const
{
    std::vector<ValuePtr> arguments;
    ValuePtr self;

    if (m_Reference.Type->GetSelf())
        self = std::move(operand);
    else
        arguments.emplace_back(std::move(operand));

    return builder.CreateCall(m_Reference, std::move(arguments), std::move(self));
}

std::ostream &llove::UDOperator<1>::Print(std::ostream &stream) const
{
    return m_Reference.Print(stream);
}

llove::UDOperator<2>::UDOperator(const FunctionReference &reference)
    : m_Reference(std::move(reference))
{
}

llove::ValuePtr llove::UDOperator<2>::operator()(
    Builder &builder,
    ValuePtr left,
    ValuePtr right) const
{
    std::vector<ValuePtr> arguments;
    ValuePtr self;

    if (m_Reference.Type->GetSelf())
    {
        self = std::move(left);
        arguments.emplace_back(std::move(right));
    }
    else
    {
        arguments.emplace_back(std::move(left));
        arguments.emplace_back(std::move(right));
    }

    return builder.CreateCall(m_Reference, std::move(arguments), std::move(self));
}

std::ostream &llove::UDOperator<2>::Print(std::ostream &stream) const
{
    return m_Reference.Print(stream);
}
