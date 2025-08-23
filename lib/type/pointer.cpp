#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::PointerType::PointerType(const TypePtr &base, const bool is_mutable)
    : m_Base(base),
      m_IsMutable(is_mutable)
{
}

llove::TypePtr llove::PointerType::GetBase() const
{
    Assert(!m_Base.expired(), "pointer type is opaque");
    return m_Base.lock();
}

bool llove::PointerType::IsMutable() const
{
    return m_IsMutable;
}

bool llove::PointerType::IsOpaque() const
{
    return m_Base.expired();
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
        m_DIType = m_Base.expired()
                       ? builder.GetDebug().GetPointerType()
                       : builder.GetDebug().GetPointerType(m_Base.lock()->GenDI(builder));

    return m_DIType;
}

llove::TypePtr llove::PointerType::Reflect(Context &context) const
{
    if (m_Base.expired())
        return context.GetPointer(m_IsMutable);

    return context.GetPointer(m_Base.lock()->Reflect(context), m_IsMutable);
}

std::string llove::PointerType::Mangle() const
{
    if (m_Base.expired())
        return 'p' + std::string(m_IsMutable ? "m" : "i") + '_';

    return 'p' + std::string(m_IsMutable ? "m" : "i") + m_Base.lock()->Mangle();
}

std::ostream &llove::PointerType::Print(std::ostream &stream) const
{
    if (m_Base.expired())
        return stream << '[' << (m_IsMutable ? "mut" : "") << ']';

    return stream << m_Base.lock() << '[' << (m_IsMutable ? "mut" : "") << ']';
}
