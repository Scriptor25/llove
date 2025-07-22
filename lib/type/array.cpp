#include <llove/builder.hpp>
#include <llove/type.hpp>

llove::ArrayType::ArrayType(TypePtr base, const unsigned size)
    : m_Base(std::move(base)),
      m_Size(size)
{
}

llove::TypePtr llove::ArrayType::GetBase() const
{
    return m_Base;
}

unsigned llove::ArrayType::GetSize() const
{
    return m_Size;
}

llove::TypeId llove::ArrayType::GetId() const
{
    return TypeId_Array;
}

bool llove::ArrayType::IsArray() const
{
    return true;
}

llvm::ArrayType *llove::ArrayType::Gen(Builder &builder) const
{
    return builder.GetArrayType(m_Base->Gen(builder), m_Size);
}

std::string llove::ArrayType::Mangle() const
{
    return 'a' + std::to_string(m_Size) + '_' + m_Base->Mangle();
}

std::ostream &llove::ArrayType::Print(std::ostream &stream) const
{
    return stream << m_Base << '[' << m_Size << ']';
}
