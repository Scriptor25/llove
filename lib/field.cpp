#include <llove/builder.hpp>
#include <llove/context.hpp>
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
            return true;
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
        error += builder.GetContext().Difference(src.GetType(), dst.GetType());
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

llove::Field::Field(const bool is_mutable, const bool is_reference, const TypePtr &type)
    : m_IsMutable(is_mutable),
      m_IsReference(is_reference),
      m_Type(type)
{
}

std::ostream &llove::Field::Print(std::ostream &stream, const bool has_name, const std::string &name) const
{
    if (has_name)
    {
        stream << (IsMutable() ? "mut " : "") << (IsReference() ? "&" : "") << name;
        if (GetType())
            stream << ": " << GetType();
        return stream;
    }
    return stream << (IsMutable() ? "mut " : "") << (IsReference() ? "&" : "") << GetType();
}

llvm::Type *llove::Field::GenIRType(Builder &builder) const
{
    const auto type = GetType()->GenIR(builder);
    return IsReference() ? builder.GetPointerType() : type;
}

llvm::DIType *llove::Field::GenDIType(Builder &builder) const
{
    const auto type = GetType()->GenDI(builder);
    return IsReference() ? builder.GetDebug().GetPointerType(type) : type;
}

llvm::Value *llove::Field::GenCast(Builder &builder, ValuePtr value, const bool unstable_ownership) const
{
    if (IsReference())
    {
        Assert(value->IsReference(), "reference from rvalue");
        Assert(GetType() == value->GetType(), "reference type mismatch");
        Assert(!IsMutable() || value->IsMutable(), "reference mutability violation");
        return value->GetPointer();
    }

    Assert(
        unstable_ownership || !GetType()->IsClass() || !value->IsReference(),
        "implicitly removing ownership from lvalue");

    value = builder.CreateCast(std::move(value), GetType(), true);
    return value->Load(builder);
}

unsigned llove::Field::SizeBits(Builder &builder) const
{
    if (IsReference())
        return 64; // TODO: target dependent
    return GetType()->SizeBits(builder);
}

std::string llove::Field::Mangle() const
{
    return std::string(IsMutable() ? "M" : "") + std::string(IsReference() ? "R" : "") + GetType()->Mangle();
}

bool llove::Field::operator==(const Field &other) const
{
    return IsReference() == other.IsReference()
           && IsMutable() == other.IsMutable()
           && GetType() == other.GetType();
}

void llove::Field::Reflect(Context &context, Field &field) const
{
    field.m_IsMutable = m_IsMutable;
    field.m_IsReference = m_IsReference;
    field.m_Type = GetType()->Reflect(context);
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
