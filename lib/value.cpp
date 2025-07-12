#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Value::CreateR(TypePtr type, llvm::Value *value)
{
    return std::make_shared<RValue>(type, value);
}

llove::ValuePtr llove::Value::CreateL(TypePtr type, llvm::Value *pointer, bool mutable_)
{
    return std::make_shared<LValue>(type, pointer, mutable_);
}

llove::TypePtr llove::Value::GetType() const
{
    return m_Type;
}

llove::Value::Value(TypePtr type)
    : m_Type(std::move(type))
{
}

llove::RValue::RValue(TypePtr type, llvm::Value *value)
    : Value(std::move(type)),
      m_Value(value)
{
}

bool llove::RValue::IsMutable() const
{
    return false;
}

llvm::Value *llove::RValue::Load(Builder &builder) const
{
    return m_Value;
}

void llove::RValue::Store(Builder &builder, llvm::Value *value, bool volatile_) const
{
    Error("cannot store value to rvalue");
}

llove::ValuePtr llove::RValue::Reference(Builder &builder) const
{
    Error("cannot reference rvalue");
}

llove::LValue::LValue(TypePtr type, llvm::Value *pointer, const bool mutable_)
    : Value(std::move(type)),
      m_Pointer(pointer),
      m_Mutable(mutable_)
{
}

bool llove::LValue::IsMutable() const
{
    return m_Mutable;
}

llvm::Value *llove::LValue::Load(Builder &builder) const
{
    return builder.CreateLoad(m_Pointer, m_Type);
}

void llove::LValue::Store(Builder &builder, llvm::Value *value, const bool volatile_) const
{
    if (!m_Mutable)
        Error("cannot store value to immutable lvalue");
    builder.CreateStore(m_Pointer, value, volatile_);
}

llove::ValuePtr llove::LValue::Reference(Builder &builder) const
{
    return CreateR(builder.GetTypes().GetPtr(m_Type, m_Mutable), m_Pointer);
}
