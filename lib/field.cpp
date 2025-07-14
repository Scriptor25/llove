#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/field.hpp>
#include <llove/value.hpp>

llvm::Type *llove::Field::Gen(Builder &builder) const
{
    return Reference ? builder.GetPointerType(Type->Gen(builder)) : Type->Gen(builder);
}

llvm::Value *llove::Field::Gen(Builder &builder, ValuePtr value, const bool strict) const
{
    if (Reference)
    {
        Assert(Type == value->GetType(), "reference type mismatch");
        Assert(!Mutable || value->IsMutable(), "reference mutability violation");

        if (strict)
        {
            Assert(value->IsReferenceable(), "reference from rvalue");
        }
        else if (!value->IsReferenceable())
        {
            const auto pointer = builder.CreateAlloca(builder.GetParent(), Type);
            builder.CreateStore(pointer, value->Load(builder));
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
