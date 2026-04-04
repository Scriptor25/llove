#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::TypeId llove::VoidType::GetId() const
{
    return ID;
}

bool llove::VoidType::IsVoid() const
{
    return true;
}

unsigned llove::VoidType::SizeBits(Builder & /* builder */)
{
    return 0;
}

llvm::Type *llove::VoidType::GenIR(Builder &builder)
{
    if (!m_IRType)
        m_IRType = builder.GetVoidType();

    return m_IRType;
}

llvm::DIType *llove::VoidType::GenDI(Builder &builder)
{
    if (!m_DIType)
        m_DIType = builder.GetDebug().GetVoidType();

    return m_DIType;
}

llove::TypePtr llove::VoidType::Reflect(Context &context) const
{
    return context.GetVoid();
}

bool llove::VoidType::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    dst.push_back(builder.GetI32(ID));
    return true;
}

std::string llove::VoidType::Mangle() const
{
    return "v";
}

std::ostream &llove::VoidType::Print(std::ostream &stream) const
{
    return stream << "void";
}
