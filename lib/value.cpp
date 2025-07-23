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

llove::Field llove::Value::AsField() const
{
    return {
        .Mutable = IsMutable(),
        .Reference = IsReferenceable(),
        .Type = GetType(),
    };
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

bool llove::RValue::IsReferenceable() const
{
    return false;
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

void llove::RValue::Store(Builder &builder, ValuePtr value, bool volatile_) const
{
    Error("cannot store value to rvalue");
}

llove::ValuePtr llove::RValue::Reference(Builder &builder) const
{
    Error("cannot reference rvalue");
}

llvm::Value *llove::RValue::GetPointer() const
{
    Error("cannot get pointer to rvalue");
}

llove::LValue::LValue(TypePtr type, llvm::Value *pointer, const bool mutable_)
    : Value(std::move(type)),
      m_Pointer(pointer),
      m_Mutable(mutable_)
{
}

bool llove::LValue::IsReferenceable() const
{
    return true;
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
    Assert(m_Mutable, "store mutability violation");
    Assert(m_Type->Gen(builder) == value->getType(), "store type mismatch");
    builder.CreateStore(m_Pointer, value, volatile_);
}

void llove::LValue::Store(Builder &builder, const ValuePtr value, const bool volatile_) const
{
    Assert(m_Mutable, "store mutability violation");
    Assert(m_Type == value->GetType(), "store type mismatch");
    builder.CreateStore(m_Pointer, value, volatile_);
}

llove::ValuePtr llove::LValue::Reference(Builder &builder) const
{
    return CreateR(builder.GetTypes().GetPointer(m_Type, m_Mutable), m_Pointer);
}

llvm::Value *llove::LValue::GetPointer() const
{
    return m_Pointer;
}
