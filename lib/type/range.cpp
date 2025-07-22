#include <llove/builder.hpp>
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

llvm::StructType *llove::RangeType::Gen(Builder &builder) const
{
    const auto entry = m_Entry->Gen(builder);
    return builder.GetStructType({ entry, entry }, true);
}

std::string llove::RangeType::Mangle() const
{
    return 'r' + m_Entry->Mangle();
}

std::ostream &llove::RangeType::Print(std::ostream &stream) const
{
    return stream << "range<" << m_Entry << '>';
}
