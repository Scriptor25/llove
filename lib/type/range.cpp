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

llvm::StructType *llove::RangeType::GenIR(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::StructType>(m_IRType);

    const auto entry = m_Entry->GenIR(builder);
    const auto type = builder.GetStructType({ entry, entry }, true);
    m_IRType = type;
    return type;
}

llvm::DIType *llove::RangeType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    const auto entry = m_Entry->GenDI(builder);
    const auto entry_size = m_Entry->SizeBits(builder);

    const auto begin = builder.GetDebug().GetFieldType("begin", entry, entry_size, 0u);
    const auto end = builder.GetDebug().GetFieldType("end", entry, entry_size, entry_size);

    return m_DIType = builder.GetDebug().GetStructType({ begin, end }, 2 * entry_size);
}

llove::TypePtr llove::RangeType::Reflect(Context &context) const
{
    TypePtr entry;
    Type::Reflect(context, m_Entry, entry);

    return context.GetRange(std::move(entry));
}

std::string llove::RangeType::Mangle() const
{
    return 'r' + m_Entry->Mangle();
}

std::ostream &llove::RangeType::Print(std::ostream &stream) const
{
    return stream << "range<" << m_Entry << '>';
}
