#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Value::CreateR(TypePtr type, llvm::Value *value)
{
    Assert(type != nullptr, "type must not be null");
    Assert(value != nullptr, "value must not be null");
    return std::make_shared<RValue>(type, value);
}

llove::ValuePtr llove::Value::CreateL(TypePtr type, llvm::Value *pointer, bool mutable_)
{
    Assert(type != nullptr, "type must not be null");
    Assert(pointer != nullptr, "pointer must not be null");
    return std::make_shared<LValue>(type, pointer, mutable_);
}

llove::TypePtr llove::Value::GetType() const
{
    return m_Type;
}

llove::Field llove::Value::AsField() const
{
    return Field(IsMutable(), IsReference(), GetType());
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

bool llove::RValue::IsReference() const
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
    Error("illegal store to rvalue");
}

void llove::RValue::Store(Builder &builder, ValuePtr value, bool volatile_) const
{
    Error("illegal store to rvalue");
}

llove::ValuePtr llove::RValue::Reference(Builder &builder) const
{
    Error("illegal reference to rvalue");
}

llvm::Value *llove::RValue::GetPointer() const
{
    Error("illegal pointer to rvalue");
}

llove::LValue::LValue(TypePtr type, llvm::Value *pointer, const bool mutable_)
    : Value(std::move(type)),
      m_Pointer(pointer),
      m_Mutable(mutable_)
{
}

bool llove::LValue::IsReference() const
{
    return true;
}

bool llove::LValue::IsMutable() const
{
    return m_Mutable;
}

llvm::Value *llove::LValue::Load(Builder &builder) const
{
    return builder.CreateLoad(m_Type->GenIR(builder), m_Pointer);
}

void llove::LValue::Store(Builder &builder, llvm::Value *value, const bool volatile_) const
{
    Assert(m_Mutable, "store mutability violation");
    Assert(m_Type->GenIR(builder) == value->getType(), "store type mismatch");
    builder.CreateStore(value, m_Pointer, volatile_);
}

void llove::LValue::Store(Builder &builder, const ValuePtr value, const bool volatile_) const
{
    Assert(m_Mutable, "store mutability violation");
    Assert(m_Type == value->GetType(), "store type mismatch");
    builder.CreateStore(value->Load(builder), m_Pointer, volatile_);
}

llove::ValuePtr llove::LValue::Reference(Builder &builder) const
{
    return CreateR(builder.GetContext().GetPointer(m_Type, m_Mutable), m_Pointer);
}

llvm::Value *llove::LValue::GetPointer() const
{
    return m_Pointer;
}
