#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::FloatType::FloatType(const unsigned bits)
    : m_Bits(bits)
{
}

unsigned llove::FloatType::GetBits() const
{
    return m_Bits;
}

llove::TypeId llove::FloatType::GetId() const
{
    return TypeId_Float;
}

bool llove::FloatType::IsFloat() const
{
    return true;
}

unsigned llove::FloatType::SizeBits(Builder &builder) const
{
    return m_Bits;
}

llvm::Type *llove::FloatType::GenIR(Builder &builder)
{
    if (!m_IRType)
        m_IRType = builder.GetFloatType(m_Bits);
    return m_IRType;
}

llvm::DIType *llove::FloatType::GenDI(Builder &builder)
{
    if (!m_DIType)
        m_DIType = builder.GetDebug().GetFloatType(m_Bits);
    return m_DIType;
}

llove::TypePtr llove::FloatType::Reflect(Context &context) const
{
    return context.GetFloat(m_Bits);
}

std::string llove::FloatType::Mangle() const
{
    return 'f' + std::to_string(m_Bits) + '_';
}

std::ostream &llove::FloatType::Print(std::ostream &stream) const
{
    return stream << 'f' << m_Bits;
}
