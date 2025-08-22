#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::IntegerType::IntegerType(const bool sign, const unsigned bits)
    : m_Sign(sign),
      m_Bits(bits)
{
}

bool llove::IntegerType::IsSigned() const
{
    return m_Sign;
}

unsigned llove::IntegerType::GetBits() const
{
    return m_Bits;
}

llove::TypeId llove::IntegerType::GetId() const
{
    return TypeId_Integer;
}

bool llove::IntegerType::IsInteger() const
{
    return true;
}

llvm::IntegerType *llove::IntegerType::GenIR(Builder &builder)
{
    if (!m_IRType)
        m_IRType = builder.GetIntegerType(m_Bits);
    return llvm::dyn_cast<llvm::IntegerType>(m_IRType);
}

llvm::DIType *llove::IntegerType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    return m_DIType = builder.GetDebug().GetIntegerType(m_Sign, m_Bits);
}

llove::TypePtr llove::IntegerType::Reflect(Context &context) const
{
    return context.GetInteger(m_Sign, m_Bits);
}

std::string llove::IntegerType::Mangle() const
{
    return (m_Sign ? 'i' : 'u') + std::to_string(m_Bits) + '_';
}

std::ostream &llove::IntegerType::Print(std::ostream &stream) const
{
    return stream << (m_Sign ? 'i' : 'u') << m_Bits;
}
