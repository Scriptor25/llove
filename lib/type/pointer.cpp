#include <llove/builder.hpp>
#include <llove/context.hpp>
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

bool llove::PointerType::IsPointer() const
{
    return true;
}

unsigned llove::PointerType::SizeBits(Builder &builder) const
{
    return 64; // TODO: target dependent
}

llvm::PointerType *llove::PointerType::GenIR(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::PointerType>(m_IRType);

    const auto type = m_Base
                          ? builder.GetPointerType(m_Base->GenIR(builder))
                          : builder.GetPointerType();
    m_IRType = type;
    return type;
}

llvm::DIType *llove::PointerType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    return m_DIType = m_Base
                          ? builder.GetDebug().GetPointerType(m_Base->GenDI(builder))
                          : builder.GetDebug().GetPointerType();
}

llove::TypePtr llove::PointerType::Reflect(Context &context) const
{
    TypePtr base;
    Type::Reflect(context, m_Base, base);

    return context.GetPointer(std::move(base), m_Mutable);
}

std::string llove::PointerType::Mangle() const
{
    if (m_Base)
        return 'p' + std::string(m_Mutable ? "m" : "i") + m_Base->Mangle();
    return 'p' + std::string(m_Mutable ? "m" : "i") + '_';
}

std::ostream &llove::PointerType::Print(std::ostream &stream) const
{
    if (m_Base)
        return stream << m_Base << '[' << (m_Mutable ? "mut" : "") << ']';
    return stream << '[' << (m_Mutable ? "mut" : "") << ']';
}
