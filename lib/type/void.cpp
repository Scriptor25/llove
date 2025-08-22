#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::TypeId llove::VoidType::GetId() const
{
    return TypeId_Void;
}

bool llove::VoidType::IsVoid() const
{
    return true;
}

unsigned llove::VoidType::SizeBits(Builder &builder)
{
    return 0;
}

llvm::Type *llove::VoidType::GenIR(Builder &builder)
{
    if (m_IRType)
        return m_IRType;

    return m_IRType = builder.GetVoidType();
}

llvm::DIType *llove::VoidType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    return m_DIType = builder.GetDebug().GetVoidType();
}

llove::TypePtr llove::VoidType::Reflect(Context &context) const
{
    return context.GetVoid();
}

std::string llove::VoidType::Mangle() const
{
    return "v";
}

std::ostream &llove::VoidType::Print(std::ostream &stream) const
{
    return stream << "void";
}
