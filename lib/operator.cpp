#include <utility>
#include <llove/builder.hpp>
#include <llove/operator.hpp>
#include <llove/value.hpp>

llove::BIOperator<1>::BIOperator(CalleeType callee, const bool suffix)
    : m_Callee(std::move(callee)),
      m_Suffix(suffix)
{
}

llove::ValuePtr llove::BIOperator<1>::operator()(Builder &builder, ValuePtr operand) const
{
    return m_Callee(builder, std::move(operand), m_Suffix);
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

llove::UDOperator<1>::UDOperator(FunctionType::Ptr type, llvm::Value *callee)
    : m_Type(std::move(type)),
      m_Callee(callee)
{
}

llove::ValuePtr llove::UDOperator<1>::operator()(Builder &builder, ValuePtr operand) const
{
    auto operand_value = (m_Type->HasSelf() ? m_Type->GetSelf() : m_Type->GetParameter(0))
            .Gen(builder, std::move(operand));

    const auto result_value = builder.CreateCall(m_Type, m_Callee, { operand_value });

    auto &[mutable_, reference_, type_] = m_Type->GetResult();

    if (reference_)
        return Value::CreateL(type_, result_value, mutable_);

    return Value::CreateR(type_, result_value);
}

llove::UDOperator<2>::UDOperator(FunctionType::Ptr type, llvm::Value *callee)
    : m_Type(std::move(type)),
      m_Callee(callee)
{
}

llove::ValuePtr llove::UDOperator<2>::operator()(
    Builder &builder,
    ValuePtr left,
    ValuePtr right) const
{
    llvm::Value *left_value, *right_value;
    if (m_Type->HasSelf())
    {
        left_value = m_Type->GetSelf().Gen(builder, std::move(left));
        right_value = m_Type->GetParameter(0).Gen(builder, std::move(right));
    }
    else
    {
        left_value = m_Type->GetParameter(0).Gen(builder, std::move(left));
        right_value = m_Type->GetParameter(1).Gen(builder, std::move(right));
    }

    const auto result_value = builder.CreateCall(m_Type, m_Callee, { left_value, right_value });

    auto &[mutable_, reference_, type_] = m_Type->GetResult();

    if (reference_)
        return Value::CreateL(type_, result_value, mutable_);

    return Value::CreateR(type_, result_value);
}
