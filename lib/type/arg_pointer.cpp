#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::TypeId llove::ArgPointerType::GetId() const
{
    return TypeId_ArgPointer;
}

bool llove::ArgPointerType::IsArgPointer() const
{
    return true;
}

unsigned llove::ArgPointerType::SizeBits(Builder &builder) const
{
    return 64; // TODO: target dependent
}

llvm::Type *llove::ArgPointerType::GenIR(Builder &builder)
{
    if (m_IRType)
        return m_IRType;
    return m_IRType = builder.GetPointerType(builder.GetArgListType());
}

llvm::DIType *llove::ArgPointerType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;
    return m_DIType = builder.GetDebug().GetPointerType();
}

llove::TypePtr llove::ArgPointerType::Reflect(Context &context) const
{
    return context.GetArgPointer();
}

std::string llove::ArgPointerType::Mangle() const
{
    return "va";
}

std::ostream &llove::ArgPointerType::Print(std::ostream &stream) const
{
    return stream << "<...>";
}
