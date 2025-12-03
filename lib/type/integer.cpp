#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::IntegerType::IntegerType(
    const bool is_signed,
    const unsigned bits)
    : m_IsSigned(is_signed),
      m_Bits(bits)
{
    Assert(bits == 1 || bits == 8 || bits == 16 || bits == 32 || bits == 64, "bits must be either 1, 8, 16, 32 or 64");
}

bool llove::IntegerType::IsSigned() const
{
    return m_IsSigned;
}

unsigned llove::IntegerType::GetBits() const
{
    return m_Bits;
}

llove::TypeId llove::IntegerType::GetId() const
{
    return ID;
}

bool llove::IntegerType::IsInteger() const
{
    return true;
}

llvm::IntegerType* llove::IntegerType::GenIR(Builder& builder)
{
    if (!m_IRType)
        m_IRType = builder.GetIntegerType(m_Bits);

    return llvm::dyn_cast<llvm::IntegerType>(m_IRType);
}

llvm::DIType* llove::IntegerType::GenDI(Builder& builder)
{
    if (!m_DIType)
        m_DIType = builder.GetDebug().GetIntegerType(m_IsSigned, m_Bits);

    return m_DIType;
}

llove::TypePtr llove::IntegerType::Reflect(Context& context) const
{
    return context.GetInteger(m_IsSigned, m_Bits);
}

bool llove::IntegerType::TypeInfo(
    Builder& builder,
    std::vector<llvm::Constant*>& dst) const
{
    dst.emplace_back(builder.GetI32(ID));
    dst.emplace_back(builder.GetI1(m_IsSigned));
    dst.emplace_back(builder.GetI32(m_Bits));
    return true;
}

std::string llove::IntegerType::Mangle() const
{
    return (m_IsSigned ? 'i' : 'u') + std::to_string(m_Bits) + '_';
}

std::ostream& llove::IntegerType::Print(std::ostream& stream) const
{
    return stream << (m_IsSigned ? 'i' : 'u') << m_Bits;
}
