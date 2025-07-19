#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/field.hpp>
#include <llove/value.hpp>

bool llove::Field::GetCastError(
    const Builder &builder,
    const Field &dst,
    const Field &src,
    unsigned &error,
    const Penalties &penalties)
{
    if (dst.Reference)
    {
        if (dst.Mutable)
        {
            if (dst.Type != src.Type)
                return true;
            if (!src.Reference)
                return true;
            if (!src.Mutable)
                return true;
            return false;
        }

        if (!src.Reference)
            error += penalties.Allocate;
    }
    else if (src.Reference)
    {
        error += penalties.NonReference;
    }

    if (dst.Type != src.Type)
    {
        if (!builder.IsCastable(src, dst))
            return true;
        error += penalties.Cast;
    }

    return false;
}

bool llove::Field::IsCastable(
    const Builder &builder,
    const Field &dst,
    const Field &src)
{
    return (!dst.Reference && builder.IsCastable(src, dst))
           || (dst.Reference && dst.Type == src.Type
               && (!dst.Mutable || (src.Reference && src.Mutable)));
}

bool llove::Field::IsAssignable(const Field &dst, const Field &src)
{
    return dst.Type == src.Type && (!dst.Reference || !dst.Mutable || (src.Reference && src.Mutable));
}

llvm::Type *llove::Field::GenType(Builder &builder) const
{
    const auto type = Type->Gen(builder);
    return Reference ? builder.GetPointerType(type) : type;
}

llvm::Value *llove::Field::GenCast(Builder &builder, ValuePtr value, const bool strict) const
{
    if (Reference)
    {
        Assert(!Mutable || value->IsMutable(), "reference mutability violation");

        if (strict)
        {
            Assert(Type == value->GetType(), "reference type mismatch");
            Assert(value->IsReferenceable(), "reference from rvalue");
        }
        else if (!value->IsReferenceable())
        {
            value = builder.CreateCast(std::move(value), Type);

            const auto pointer = builder.CreateAlloca(Type);
            builder.CreateStore(pointer, value);

            value = Value::CreateL(Type, pointer, false);
        }

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
