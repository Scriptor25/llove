#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::RangeType::RangeType(TypePtr entry)
    : m_Entry(std::move(entry))
{
    Assert(m_Entry != nullptr, "entry must not be null");
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

llvm::StructType* llove::RangeType::GenIR(Builder& builder)
{
    if (!m_IRType)
    {
        const auto entry_type = m_Entry->GenIR(builder);
        m_IRType = builder.GetStructType({ entry_type, entry_type }, false);
    }

    return llvm::dyn_cast<llvm::StructType>(m_IRType);
}

llvm::DIType* llove::RangeType::GenDI(Builder& builder)
{
    if (!m_DIType)
    {
        const auto entry_type = m_Entry->GenDI(builder);
        const auto entry_size = m_Entry->SizeBits(builder);

        const auto begin = builder.GetDebug().GetFieldType("begin", entry_type, entry_size, 0u);
        const auto end = builder.GetDebug().GetFieldType("end", entry_type, entry_size, entry_size);

        m_DIType = builder.GetDebug().GetStructType({ begin, end }, 2 * entry_size);
    }

    return m_DIType;
}

llove::TypePtr llove::RangeType::Reflect(Context& context) const
{
    return context.GetRange(m_Entry->Reflect(context));
}

bool llove::RangeType::TypeInfo(
    Builder& builder,
    std::vector<llvm::Constant*>& dst) const
{
    dst.emplace_back(builder.GetI32(ID));
    return m_Entry->TypeInfo(builder, dst);
}

std::string llove::RangeType::Mangle() const
{
    return 'r' + m_Entry->Mangle();
}

std::ostream& llove::RangeType::Print(std::ostream& stream) const
{
    return stream << "range<" << m_Entry << '>';
}
