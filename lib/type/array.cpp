#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::ArrayType::ArrayType(TypePtr base, const unsigned count)
    : m_Base(std::move(base)),
      m_Count(count)
{
}

llove::TypePtr llove::ArrayType::GetBase() const
{
    Assert(!m_Base.expired(), "base has expired");
    return m_Base.lock();
}

unsigned llove::ArrayType::GetCount() const
{
    return m_Count;
}

llove::TypeId llove::ArrayType::GetId() const
{
    return TypeId_Array;
}

bool llove::ArrayType::IsArray() const
{
    return true;
}

llvm::ArrayType *llove::ArrayType::GenIR(Builder &builder)
{
    Assert(!m_Base.expired(), "base has expired");
    const auto base = m_Base.lock();

    if (!m_IRType)
        m_IRType = builder.GetArrayType(base->GenIR(builder), m_Count);

    return llvm::dyn_cast<llvm::ArrayType>(m_IRType);
}

llvm::DIType *llove::ArrayType::GenDI(Builder &builder)
{
    Assert(!m_Base.expired(), "base has expired");
    const auto base = m_Base.lock();

    if (!m_DIType)
        m_DIType = builder.GetDebug().GetArrayType(base->GenDI(builder), m_Count);

    return m_DIType;
}

llove::TypePtr llove::ArrayType::Reflect(Context &context) const
{
    Assert(!m_Base.expired(), "base has expired");
    const auto base = m_Base.lock();

    return context.GetArray(base->Reflect(context), m_Count);
}

std::string llove::ArrayType::Mangle() const
{
    Assert(!m_Base.expired(), "base has expired");
    const auto base = m_Base.lock();

    return 'a' + std::to_string(m_Count) + '_' + base->Mangle();
}

std::ostream &llove::ArrayType::Print(std::ostream &stream) const
{
    Assert(!m_Base.expired(), "base has expired");
    const auto base = m_Base.lock();

    return stream << base << '[' << m_Count << ']';
}
