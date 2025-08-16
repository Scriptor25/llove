#include <llove/builder.hpp>
#include <llove/context.hpp>
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

unsigned llove::ArrayType::SizeBits(Builder &builder) const
{
    return m_Size * m_Base->SizeBits(builder);
}

llvm::ArrayType *llove::ArrayType::GenIR(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::ArrayType>(m_IRType);

    const auto type = builder.GetArrayType(m_Base->GenIR(builder), m_Size);
    m_IRType = type;
    return type;
}

llvm::DIType *llove::ArrayType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    return m_DIType = builder.GetDebug().GetArrayType(m_Base->GenDI(builder), m_Size);
}

llove::TypePtr llove::ArrayType::Reflect(Context &context) const
{
    TypePtr base;
    Type::Reflect(context, m_Base, base);

    return context.GetArray(std::move(base), m_Size);
}

std::string llove::ArrayType::Mangle() const
{
    return 'a' + std::to_string(m_Size) + '_' + m_Base->Mangle();
}

std::ostream &llove::ArrayType::Print(std::ostream &stream) const
{
    return stream << m_Base << '[' << m_Size << ']';
}
