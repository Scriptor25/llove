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

    if (dst.Type->GetId() == TypeId_Class && src.Reference)
        return true;
    if (dst.Type != src.Type)
    {
        if (strict || !builder.IsCastable(src, dst))
            return true;
        error += 5u;
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

    if (dst.Type->GetId() == TypeId_Class && src.Reference)
        return false;
    if (dst.Type != src.Type)
        if (strict || !builder.IsCastable(src, dst))
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

llvm::Value *llove::Field::GenCast(Builder &builder, ValuePtr value) const
{
    if (Reference)
    {
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(Type == value->GetType(), "reference type mismatch");
        Assert(!Mutable || value->IsMutable(), "reference mutability violation");
        return value->GetPointer();
    }

    value = builder.CreateCast(std::move(value), Type);
    return value->Load(builder);
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

llove::Field::operator bool() const
{
    return Type != nullptr;
}

std::string llove::GetFieldHash(const std::vector<Field> &fields)
{
    std::string hash;
    hash += std::to_string(fields.size());
    for (const auto &[mutable_, reference_, type_] : fields)
    {
        if (mutable_)
            hash += 'm';
        if (reference_)
            hash += 'r';
        hash += std::to_string(reinterpret_cast<uintptr_t>(type_.get()));
    }
    return hash;
}
