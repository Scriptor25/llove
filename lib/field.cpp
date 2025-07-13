#include <llove/builder.hpp>
#include <llove/field.hpp>

llvm::Type *llove::Field::Gen(Builder &builder) const
{
    return Reference ? builder.GetPointerType(Type->Gen(builder)) : Type->Gen(builder);
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
