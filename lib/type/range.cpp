#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::RangeType::RangeType(TypePtr entry)
    : m_Entry(std::move(entry))
{
}

llove::TypePtr llove::RangeType::GetEntry() const
{
    return m_Entry;
}

llove::TypeId llove::RangeType::GetId() const
{
    return TypeId_Range;
}

bool llove::RangeType::IsRange() const
{
    return true;
}

unsigned llove::RangeType::SizeBits(Builder &builder) const
{
    return 2 * m_Entry->SizeBits(builder);
}

llvm::StructType *llove::RangeType::Gen(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::StructType>(m_IRType);

    const auto entry = m_Entry->Gen(builder);
    const auto type = builder.GetStructType({ entry, entry }, true);
    m_IRType = type;
    return type;
}

llvm::DIType *llove::RangeType::GenDbg(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    const auto entry = m_Entry->GenDbg(builder);
    return m_DIType = builder.GetDbgStructType({ entry, entry }, SizeBits(builder));
}

llove::TypePtr llove::RangeType::Reflect(Builder &builder) const
{
    TypePtr entry;
    if (m_Entry)
        m_Entry->Reflect(builder, entry);

    return builder.GetTypes().GetRange(std::move(entry));
}

std::string llove::RangeType::Mangle() const
{
    return 'r' + m_Entry->Mangle();
}

std::ostream &llove::RangeType::Print(std::ostream &stream) const
{
    return stream << "range<" << m_Entry << '>';
}
