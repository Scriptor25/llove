#include <llove/builder.hpp>
#include <llove/type.hpp>

llove::TypeId llove::VoidType::GetId() const
{
    return TypeId_Void;
}

llvm::Type *llove::VoidType::Gen(Builder &builder) const
{
    return builder.GetVoidType();
}

std::string llove::VoidType::Mangle() const
{
    return "v";
}

std::ostream &llove::VoidType::Print(std::ostream &stream) const
{
    return stream << "void";
}
