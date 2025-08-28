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
    if (dst.IsReference())
    {
        if (!src.IsReference())
            return true;
        if (dst.GetType() != src.GetType())
        {
            if (!(src.GetType()->IsClass() && As<ClassType>(src.GetType())->InheritsFrom(dst.GetType())))
                return true;
            error += 1u;
        }
        if (dst.IsMutable() && !src.IsMutable())
            return true;
        if (dst.IsMutable() != src.IsMutable())
            error += 1u;
        return false;
    }

    if (dst.GetType()->IsClass() && src.IsReference())
        return true;
    if (dst.GetType() != src.GetType())
    {
        if (strict || !builder.IsCastable(src, dst, true))
            return true;
        error += Difference(src.GetType(), dst.GetType());
    }

    return false;
}

bool llove::Field::IsCastable(
    const Builder &builder,
    const Field &dst,
    const Field &src,
    const bool strict)
{
    if (dst.IsReference())
    {
        if (!src.IsReference())
            return false;
        if (dst.GetType() != src.GetType())
            if (!(src.GetType()->IsClass() && As<ClassType>(src.GetType())->InheritsFrom(dst.GetType())))
                return false;
        if (dst.IsMutable() && !src.IsMutable())
            return false;
        return true;
    }

    if (dst.GetType()->IsClass() && src.IsReference())
        return false;
    if (dst.GetType() != src.GetType())
        if (strict || !builder.IsCastable(src, dst, true))
            return false;
    return true;
}

llove::Field::Field(TypePtr type)
    : m_Type(std::move(type))
{
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
        const auto type = value->GetType();

        Assert(value->IsReference(), "reference from rvalue");
        Assert(
            type == m_Type || (type->IsClass() && As<ClassType>(type)->InheritsFrom(m_Type)),
            "reference type mismatch");
        Assert(value->IsMutable() || !m_IsMutable, "reference mutability violation");
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
        return builder.GetDataLayout().getPointerSizeInBits();
    return m_Type->SizeBits(builder);
}

std::string llove::Field::Mangle() const
{
    return std::string(m_IsMutable ? "M" : "") + std::string(m_IsReference ? "R" : "") + m_Type->Mangle();
}

bool llove::Field::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    // reference gets discarded
    return m_Type->TypeInfo(builder, dst);
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
