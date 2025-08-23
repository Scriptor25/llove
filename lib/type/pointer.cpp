#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::PointerType::PointerType(const bool is_mutable)
    : m_Base(nullptr),
      m_IsMutable(is_mutable)
{
}

llove::PointerType::PointerType(TypePtr base, const bool is_mutable)
    : m_Base(std::move(base)),
      m_IsMutable(is_mutable)
{
    Assert(m_Base != nullptr, "base must not be null");
}

llove::TypePtr llove::PointerType::GetBase() const
{
    Assert(m_Base != nullptr, "pointer type is opaque");
    return m_Base;
}

bool llove::PointerType::IsMutable() const
{
    return m_IsMutable;
}

bool llove::PointerType::IsOpaque() const
{
    return !m_Base;
}

llove::TypeId llove::PointerType::GetId() const
{
    return TypeId_Pointer;
}

bool llove::PointerType::IsPointer() const
{
    return true;
}

llvm::PointerType *llove::PointerType::GenIR(Builder &builder)
{
    if (!m_IRType)
        m_IRType = builder.GetPointerType();

    return llvm::dyn_cast<llvm::PointerType>(m_IRType);
}

llvm::DIType *llove::PointerType::GenDI(Builder &builder)
{
    if (!m_DIType)
        m_DIType = m_Base
                       ? builder.GetDebug().GetPointerType(m_Base->GenDI(builder))
                       : builder.GetDebug().GetPointerType();

    return m_DIType;
}

llove::TypePtr llove::PointerType::Reflect(Context &context) const
{
    if (m_Base)
        return context.GetPointer(m_Base->Reflect(context), m_IsMutable);

    return context.GetPointer(m_IsMutable);
}

std::string llove::PointerType::Mangle() const
{
    if (m_Base)
        return 'p' + std::string(m_IsMutable ? "m" : "i") + m_Base->Mangle();

    return 'p' + std::string(m_IsMutable ? "m" : "i") + '_';
}

std::ostream &llove::PointerType::Print(std::ostream &stream) const
{
    if (m_Base)
        return stream << m_Base << '[' << (m_IsMutable ? "mut" : "") << ']';

    return stream << '[' << (m_IsMutable ? "mut" : "") << ']';
}
