#include <llove/builder.hpp>
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

llvm::IntegerType *llove::IntegerType::Gen(Builder &builder) const
{
    return builder.GetIntType(m_Bits);
}

std::string llove::IntegerType::Mangle() const
{
    return (m_Sign ? 'i' : 'u') + std::to_string(m_Bits) + '_';
}

std::ostream &llove::IntegerType::Print(std::ostream &stream) const
{
    return stream << (m_Sign ? 'i' : 'u') << m_Bits;
}
