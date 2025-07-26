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

unsigned llove::FloatType::Size(Builder &builder) const
{
    return m_Bits >> 3;
}

llvm::Type *llove::FloatType::Gen(Builder &builder) const
{
    return builder.GetFltType(m_Bits);
}

llove::TypePtr llove::FloatType::Reflect(Builder &builder) const
{
    return builder.GetTypes().GetFloat(m_Bits);
}

std::string llove::FloatType::Mangle() const
{
    return 'f' + std::to_string(m_Bits) + '_';
}

std::ostream &llove::FloatType::Print(std::ostream &stream) const
{
    return stream << 'f' << m_Bits;
}
