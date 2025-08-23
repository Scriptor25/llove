#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::RangeType::RangeType(const TypePtr &entry)
    : m_Entry(entry)
{
}

llove::TypePtr llove::RangeType::GetEntry() const
{
    Assert(!m_Entry.expired(), "entry has expired");
    return m_Entry.lock();
}

llove::TypeId llove::RangeType::GetId() const
{
    return TypeId_Range;
}

bool llove::RangeType::IsRange() const
{
    return true;
}

llvm::StructType *llove::RangeType::GenIR(Builder &builder)
{
    Assert(!m_Entry.expired(), "entry has expired");
    const auto entry = m_Entry.lock();

    if (!m_IRType)
    {
        const auto entry_type = entry->GenIR(builder);
        m_IRType = builder.GetStructType({ entry_type, entry_type }, false);
    }

    return llvm::dyn_cast<llvm::StructType>(m_IRType);
}

llvm::DIType *llove::RangeType::GenDI(Builder &builder)
{
    Assert(!m_Entry.expired(), "entry has expired");
    const auto entry = m_Entry.lock();

    if (!m_DIType)
    {
        const auto entry_type = entry->GenDI(builder);
        const auto entry_size = entry->SizeBits(builder);

        const auto begin = builder.GetDebug().GetFieldType("begin", entry_type, entry_size, 0u);
        const auto end = builder.GetDebug().GetFieldType("end", entry_type, entry_size, entry_size);

        m_DIType = builder.GetDebug().GetStructType({ begin, end }, 2 * entry_size);
    }

    return m_DIType;
}

llove::TypePtr llove::RangeType::Reflect(Context &context) const
{
    Assert(!m_Entry.expired(), "entry has expired");
    const auto entry = m_Entry.lock();

    return context.GetRange(entry->Reflect(context));
}

std::string llove::RangeType::Mangle() const
{
    Assert(!m_Entry.expired(), "entry has expired");
    const auto entry = m_Entry.lock();

    return 'r' + entry->Mangle();
}

std::ostream &llove::RangeType::Print(std::ostream &stream) const
{
    Assert(!m_Entry.expired(), "entry has expired");
    const auto entry = m_Entry.lock();

    return stream << "range<" << entry << '>';
}
