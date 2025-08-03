#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::StructType::StructType(std::vector<Parameter> fields)
    : m_Fields(std::move(fields))
{
}

bool llove::StructType::HasField(const std::string &name) const
{
    return std::ranges::any_of(
        m_Fields,
        [&name](auto &field)
        {
            return field.Name == name;
        });
}

unsigned llove::StructType::GetFieldIndex(const std::string &name) const
{
    for (unsigned i = 0; i < m_Fields.size(); ++i)
        if (m_Fields.at(i).Name == name)
            return i;
    Error("no field with name '{}'", name);
}

unsigned llove::StructType::GetFieldCount() const
{
    return m_Fields.size();
}

const llove::Field &llove::StructType::GetField(const unsigned index) const
{
    return m_Fields.at(index).Info;
}

llove::TypeId llove::StructType::GetId() const
{
    return TypeId_Struct;
}

bool llove::StructType::IsStruct() const
{
    return true;
}

unsigned llove::StructType::SizeBits(Builder &builder) const
{
    auto size = 0u;
    for (auto &field : m_Fields)
        size += field.Info.SizeBits(builder);
    return size;
}

llvm::StructType *llove::StructType::GenIR(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::StructType>(m_IRType);

    std::vector<llvm::Type *> fields;
    for (auto &field : m_Fields)
        fields.emplace_back(field.Info.GenIRType(builder));

    // TODO: packed struct
    const auto type = builder.GetStructType(fields, true);
    m_IRType = type;
    return type;
}

llvm::DIType *llove::StructType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    std::vector<llvm::Metadata *> fields;

    auto offset = 0u;
    for (auto &field : m_Fields)
    {
        const auto field_size = field.Info.SizeBits(builder);
        fields.emplace_back(
            builder.GetDebug().GetFieldType(
                field.Name,
                field.Info.GenDIType(builder),
                field_size,
                offset));
        offset += field_size;
    }

    return m_DIType = builder.GetDebug().GetStructType(fields, offset);
}

llove::TypePtr llove::StructType::Reflect(Builder &builder) const
{
    std::vector<Parameter> fields(m_Fields.size());
    for (unsigned i = 0; i < m_Fields.size(); ++i)
    {
        fields.at(i).Name = m_Fields.at(i).Name;
        m_Fields.at(i).Info.Reflect(builder, fields.at(i).Info);
    }

    return builder.GetTypes().GetStruct(std::move(fields));
}

std::string llove::StructType::Mangle() const
{
    std::string fields;
    for (auto &field : m_Fields)
        fields += field.Info.Mangle();
    return 's' + std::to_string(m_Fields.size()) + '_' + fields;
}

std::ostream &llove::StructType::Print(std::ostream &stream) const
{
    stream << "{ ";
    for (auto i = m_Fields.begin(); i != m_Fields.end(); ++i)
    {
        if (i != m_Fields.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << " }";
}
