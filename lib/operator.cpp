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
    llvm::Value *operand_value;

    if (m_Type->HasSelf())
    {
        auto &self = m_Type->GetSelf();

        if (!operand->IsReferenceable())
        {
            const auto pointer = builder.CreateAlloca(builder.GetParent(), self.Type);
            builder.CreateStore(pointer, operand->Load(builder));
            operand = Value::CreateL(self.Type, pointer, false);
        }
        operand_value = operand->GetPointer();
    }
    else
    {
        if (auto &[mutable_, reference_, type_] = m_Type->GetParameter(0); reference_)
        {
            if (!operand->IsReferenceable())
            {
                const auto pointer = builder.CreateAlloca(builder.GetParent(), type_);
                builder.CreateStore(pointer, operand->Load(builder));
                operand = Value::CreateL(type_, pointer, false);
            }
            operand_value = operand->GetPointer();
        }
        else
        {
            operand = builder.CreateCast(operand, type_);
            operand_value = operand->Load(builder);
        }
    }

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
    llvm::Value *left_value;
    llvm::Value *right_value;

    if (m_Type->HasSelf())
    {
        auto &self = m_Type->GetSelf();

        if (!left->IsReferenceable())
        {
            const auto pointer = builder.CreateAlloca(builder.GetParent(), self.Type);
            builder.CreateStore(pointer, left->Load(builder));
            left = Value::CreateL(self.Type, pointer, false);
        }
        left_value = left->GetPointer();

        if (auto &[mutable_, reference_, type_] = m_Type->GetParameter(0); reference_)
        {
            if (!right->IsReferenceable())
            {
                const auto pointer = builder.CreateAlloca(builder.GetParent(), type_);
                builder.CreateStore(pointer, right->Load(builder));
                right = Value::CreateL(type_, pointer, false);
            }
            right_value = right->GetPointer();
        }
        else
        {
            right = builder.CreateCast(right, type_);
            right_value = right->Load(builder);
        }
    }
    else
    {
        if (auto &[mutable_, reference_, type_] = m_Type->GetParameter(0); reference_)
        {
            if (!left->IsReferenceable())
            {
                const auto pointer = builder.CreateAlloca(builder.GetParent(), type_);
                builder.CreateStore(pointer, left->Load(builder));
                left = Value::CreateL(type_, pointer, false);
            }
            left_value = left->GetPointer();
        }
        else
        {
            left = builder.CreateCast(left, type_);
            left_value = left->Load(builder);
        }

        if (auto &[mutable_, reference_, type_] = m_Type->GetParameter(1); reference_)
        {
            if (!right->IsReferenceable())
            {
                const auto pointer = builder.CreateAlloca(builder.GetParent(), type_);
                builder.CreateStore(pointer, right->Load(builder));
                right = Value::CreateL(type_, pointer, false);
            }
            right_value = right->GetPointer();
        }
        else
        {
            right = builder.CreateCast(right, type_);
            right_value = right->Load(builder);
        }
    }

    const auto result_value = builder.CreateCall(m_Type, m_Callee, { left_value, right_value });

    auto &[mutable_, reference_, type_] = m_Type->GetResult();
    if (reference_)
        return Value::CreateL(type_, result_value, mutable_);
    return Value::CreateR(type_, result_value);
}
