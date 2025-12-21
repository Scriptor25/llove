#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::ArrayType::ArrayType(
    TypePtr base,
    const unsigned count)
    : m_Base(std::move(base)),
      m_Count(count)
{
    Assert(m_Base != nullptr, "base must not be null");
}

llove::TypePtr llove::ArrayType::GetBase() const
{
    return m_Base;
}

unsigned llove::ArrayType::GetCount() const
{
    return m_Count;
}

llove::TypeId llove::ArrayType::GetId() const
{
    return ID;
}

bool llove::ArrayType::IsArray() const
{
    return true;
}

llvm::ArrayType* llove::ArrayType::GenIR(Builder& builder)
{
    if (!m_IRType)
        m_IRType = builder.GetArrayType(m_Base->GenIR(builder), m_Count);

    return llvm::dyn_cast<llvm::ArrayType>(m_IRType);
}

llvm::DIType* llove::ArrayType::GenDI(Builder& builder)
{
    if (!m_DIType)
        m_DIType = builder.GetDebug().GetArrayType(m_Base->GenDI(builder), m_Count);

    return m_DIType;
}

llove::TypePtr llove::ArrayType::Reflect(Context& context) const
{
    TypePtr base;
    Type::Reflect(context, m_Base, base);
    return context.GetArray(std::move(base), m_Count);
}

bool llove::ArrayType::TypeInfo(
    Builder& builder,
    std::vector<llvm::Constant*>& dst) const
{
    dst.emplace_back(builder.GetI32(ID));
    dst.emplace_back(builder.GetI32(m_Count));
    return m_Base->TypeInfo(builder, dst);
}

std::string llove::ArrayType::Mangle() const
{
    return 'a' + std::to_string(m_Count) + '_' + m_Base->Mangle();
}

std::ostream& llove::ArrayType::Print(std::ostream& stream) const
{
    return stream << m_Base << '[' << m_Count << ']';
}
