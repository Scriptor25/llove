#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/field.hpp>
#include <llove/value.hpp>

bool llove::Field::GetCastError(
    const Builder &builder,
    const Field &dst,
    const Field &src,
    unsigned &error,
    const bool strict)
{
    if (dst.m_IsReference)
    {
        if (!src.m_IsReference)
            return true;
        if (dst.m_Type != src.m_Type)
            return true;
        if (dst.m_IsMutable && !src.m_IsMutable)
            return true;
        if (dst.m_IsMutable != src.m_IsMutable)
            error += 1u;
        return false;
    }

    if (dst.m_Type->IsClass() && src.m_IsReference)
        return true;
    if (dst.m_Type != src.m_Type)
    {
        if (strict || !builder.IsCastable(src, dst, true))
            return true;
        error += Difference(src.m_Type, dst.m_Type);
    }

    return false;
}

bool llove::Field::IsCastable(
    const Builder &builder,
    const Field &dst,
    const Field &src,
    const bool strict)
{
    if (dst.m_IsReference)
    {
        if (!src.m_IsReference)
            return false;
        if (dst.m_Type != src.m_Type)
            return false;
        if (dst.m_IsMutable && !src.m_IsMutable)
            return false;
        return true;
    }

    if (dst.m_Type->IsClass() && src.m_IsReference)
        return false;
    if (dst.m_Type != src.m_Type)
        if (strict || !builder.IsCastable(src, dst, true))
            return false;
    return true;
}

llove::Field::Field(const bool is_mutable, const bool is_reference, TypePtr type)
    : m_IsMutable(is_mutable),
      m_IsReference(is_reference),
      m_Type(std::move(type))
{
}

bool llove::Field::IsMutable() const
{
    return m_IsMutable;
}

bool llove::Field::IsReference() const
{
    return m_IsReference;
}

bool llove::Field::HasType() const
{
    return m_Type != nullptr;
}

llove::TypePtr llove::Field::GetType() const
{
    Assert(m_Type != nullptr, "field does not have a type");
    return m_Type;
}

llove::Field &llove::Field::SetIsMutable(const bool is_mutable)
{
    m_IsMutable = is_mutable;
    return *this;
}

llove::Field &llove::Field::SetIsReference(const bool is_reference)
{
    m_IsReference = is_reference;
    return *this;
}

llove::Field &llove::Field::SetType(TypePtr type)
{
    m_Type = std::move(type);
    return *this;
}

std::ostream &llove::Field::Print(std::ostream &stream, const bool has_name, const std::string &name) const
{
    if (has_name)
    {
        stream << (m_IsMutable ? "mut " : "") << (m_IsReference ? "&" : "") << name;
        if (m_Type)
            stream << ": " << m_Type;
        return stream;
    }
    return stream << (m_IsMutable ? "mut " : "") << (m_IsReference ? "&" : "") << m_Type;
}

llvm::Type *llove::Field::GenIRType(Builder &builder) const
{
    const auto type = m_Type->GenIR(builder);
    return m_IsReference ? builder.GetPointerType() : type;
}

llvm::DIType *llove::Field::GenDIType(Builder &builder) const
{
    const auto type = m_Type->GenDI(builder);
    return m_IsReference ? builder.GetDebug().GetPointerType(type) : type;
}

llvm::Value *llove::Field::GenCast(Builder &builder, ValuePtr value, const bool unstable_ownership) const
{
    if (m_IsReference)
    {
        Assert(value->IsReference(), "reference from rvalue");
        Assert(m_Type == value->GetType(), "reference type mismatch");
        Assert(!m_IsMutable || value->IsMutable(), "reference mutability violation");
        return value->GetPointer();
    }

    Assert(
        unstable_ownership || !m_Type->IsClass() || !value->IsReference(),
        "implicitly removing ownership from lvalue");

    value = builder.CreateCast(std::move(value), m_Type, true);
    return value->Load(builder);
}

unsigned llove::Field::SizeBits(Builder &builder) const
{
    if (m_IsReference)
        return 64; // TODO: target dependent
    return m_Type->SizeBits(builder);
}

std::string llove::Field::Mangle() const
{
    return std::string(m_IsMutable ? "M" : "") + std::string(m_IsReference ? "R" : "") + m_Type->Mangle();
}

bool llove::Field::operator==(const Field &other) const
{
    return m_IsReference == other.m_IsReference
           && m_IsMutable == other.m_IsMutable
           && m_Type == other.m_Type;
}

void llove::Field::Reflect(Context &context, Field &field) const
{
    field.m_IsMutable = m_IsMutable;
    field.m_IsReference = m_IsReference;
    Type::Reflect(context, m_Type, field.m_Type);
}

std::string llove::GetFieldHash(const std::vector<Field> &fields)
{
    std::string hash;
    hash += std::to_string(fields.size());
    for (auto &field : fields)
    {
        if (field.IsMutable())
            hash += 'm';
        if (field.IsReference())
            hash += 'r';
        hash += std::to_string(reinterpret_cast<uintptr_t>(field.GetType().get()));
    }
    return hash;
}
