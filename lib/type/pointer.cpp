#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::PointerType::PointerType(TypePtr base, const bool mutable_)
    : m_Base(std::move(base)),
      m_Mutable(mutable_)
{
}

llove::TypePtr llove::PointerType::GetBase() const
{
    Assert(m_Base != nullptr, "pointer type is opaque");
    return m_Base;
}

bool llove::PointerType::IsMutable() const
{
    return m_Mutable;
}

bool llove::PointerType::IsOpaque() const
{
    return !m_Base;
}

llove::TypeId llove::PointerType::GetId() const
{
    return TypeId_Pointer;
}

llvm::PointerType *llove::PointerType::Gen(Builder &builder) const
{
    if (m_Base)
        return builder.GetPointerType(m_Base->Gen(builder));
    return builder.GetPointerType();
}

std::string llove::PointerType::Mangle() const
{
    return 'p' + std::string(m_Mutable ? "m" : "i") + m_Base->Mangle();
}

std::ostream &llove::PointerType::Print(std::ostream &stream) const
{
    if (m_Base)
        return stream << m_Base << '[' << (m_Mutable ? "mut" : "") << ']';
    return stream << '[' << (m_Mutable ? "mut" : "") << ']';
}
