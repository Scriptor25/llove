#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::FloatType::FloatType(const unsigned bits)
    : m_Bits(bits)
{
    Assert(m_Bits == 16 || m_Bits == 32 || m_Bits == 64, "bits must be either 16, 32 or 64");
}

unsigned llove::FloatType::GetBits() const
{
    return m_Bits;
}

llove::TypeId llove::FloatType::GetId() const
{
    return ID;
}

bool llove::FloatType::IsFloat() const
{
    return true;
}

llvm::Type* llove::FloatType::GenIR(Builder& builder)
{
    if (!m_IRType)
        m_IRType = builder.GetFloatType(m_Bits);
    return m_IRType;
}

llvm::DIType* llove::FloatType::GenDI(Builder& builder)
{
    if (!m_DIType)
        m_DIType = builder.GetDebug().GetFloatType(m_Bits);
    return m_DIType;
}

llove::TypePtr llove::FloatType::Reflect(Context& context) const
{
    return context.GetFloat(m_Bits);
}

bool llove::FloatType::TypeInfo(
    Builder& builder,
    std::vector<llvm::Constant*>& dst) const
{
    dst.emplace_back(builder.GetI32(ID));
    dst.emplace_back(builder.GetI32(m_Bits));
    return true;
}

std::string llove::FloatType::Mangle() const
{
    return 'f' + std::to_string(m_Bits) + '_';
}

std::ostream& llove::FloatType::Print(std::ostream& stream) const
{
    return stream << 'f' << m_Bits;
}
