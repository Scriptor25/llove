#include <llove/builder.hpp>
#include <llove/operator.hpp>
#include <llove/value.hpp>

llove::BuiltinOperator::BuiltinOperator(CalleeType callee)
    : m_Callee(std::move(callee))
{
}

llove::ValuePtr llove::BuiltinOperator::operator()(
    Builder &builder,
    ValuePtr left,
    ValuePtr right) const
{
    return m_Callee(builder, std::move(left), std::move(right));
}

llove::UserDefinedOperator::UserDefinedOperator(FunctionType::Ptr type, llvm::Value *callee)
    : m_Type(std::move(type)),
      m_Callee(callee)
{
}

llove::ValuePtr llove::UserDefinedOperator::operator()(
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
