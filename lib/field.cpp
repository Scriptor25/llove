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
    if (dst.Reference)
    {
        if (!src.Reference)
            return true;
        if (dst.Type != src.Type)
            return true;
        if (dst.Mutable && !src.Mutable)
            return true;
        if (dst.Mutable != src.Mutable)
            error += 1u;
        return false;
    }

    if (dst.Type->IsClass() && src.Reference)
        return true;
    if (dst.Type != src.Type)
    {
        if (strict || !builder.IsCastable(src, dst, true))
            return true;
        error += builder.GetTypes().Difference(dst.Type, src.Type);
    }

    return false;
}

bool llove::Field::IsCastable(
    const Builder &builder,
    const Field &dst,
    const Field &src,
    const bool strict)
{
    if (dst.Reference)
    {
        if (!src.Reference)
            return false;
        if (dst.Type != src.Type)
            return false;
        if (dst.Mutable && !src.Mutable)
            return false;
        return true;
    }

    if (dst.Type->IsClass() && src.Reference)
        return false;
    if (dst.Type != src.Type)
        if (strict || !builder.IsCastable(src, dst, true))
            return false;
    return true;
}

std::ostream &llove::Field::Print(std::ostream &stream, const bool has_name, const std::string &name) const
{
    if (has_name)
    {
        stream << (Mutable ? "mut " : "") << (Reference ? "&" : "") << name;
        if (Type)
            stream << ": " << Type;
        return stream;
    }
    return stream << (Mutable ? "mut " : "") << (Reference ? "&" : "") << Type;
}

llvm::Type *llove::Field::GenType(Builder &builder) const
{
    const auto type = Type->Gen(builder);
    return Reference ? builder.GetPointerType(type) : type;
}

llvm::DIType *llove::Field::GenDbgType(Builder &builder) const
{
    const auto type = Type->GenDbg(builder);
    return Reference ? builder.GetDbgPointerType(type) : type;
}

llvm::Value *llove::Field::GenCast(Builder &builder, ValuePtr value, const bool unstable_ownership) const
{
    if (Reference)
    {
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(Type == value->GetType(), "reference type mismatch");
        Assert(!Mutable || value->IsMutable(), "reference mutability violation");
        return value->GetPointer();
    }

    Assert(
        unstable_ownership || !Type->IsClass() || !value->IsReferenceable(),
        "implicitly removing ownership from lvalue");

    value = builder.CreateCast(std::move(value), Type, true);
    return value->Load(builder);
}

unsigned llove::Field::SizeBits(Builder &builder) const
{
    if (Reference)
        return 64;
    return Type->SizeBits(builder);
}

std::string llove::Field::Mangle() const
{
    return std::string(Mutable ? "M" : "") + std::string(Reference ? "R" : "") + Type->Mangle();
}

bool llove::Field::operator==(const Field &other) const
{
    return Reference == other.Reference
           && Mutable == other.Mutable
           && Type == other.Type;
}

void llove::Field::Reflect(Builder &builder, Field &field) const
{
    field.Mutable = Mutable;
    field.Reference = Reference;

    if (Type)
        Type->Reflect(builder, field.Type);
}

std::string llove::GetFieldHash(const std::vector<Field> &fields)
{
    std::string hash;
    hash += std::to_string(fields.size());
    for (auto &field : fields)
    {
        if (field.Mutable)
            hash += 'm';
        if (field.Reference)
            hash += 'r';
        hash += std::to_string(reinterpret_cast<uintptr_t>(field.Type.get()));
    }
    return hash;
}
