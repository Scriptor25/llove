#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::StructType::StructType(std::vector<Parameter> fields)
    : m_Fields(std::move(fields))
{
    Assert(!m_Fields.empty(), "fields must not be empty");
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

llvm::StructType *llove::StructType::GenIR(Builder &builder)
{
    if (!m_IRType)
    {
        std::vector<llvm::Type *> fields;
        for (auto &field : m_Fields)
            fields.emplace_back(field.Info.GenIRType(builder));

        m_IRType = builder.GetStructType(fields, false);
    }

    return llvm::dyn_cast<llvm::StructType>(m_IRType);
}

llvm::DIType *llove::StructType::GenDI(Builder &builder)
{
    if (!m_DIType)
    {
        const auto layout = builder.GetDataLayout().getStructLayout(GenIR(builder));

        std::vector<llvm::Metadata *> elements;

        for (unsigned i = 0; i < m_Fields.size(); ++i)
        {
            auto &field = m_Fields.at(i);
            const auto size = field.Info.SizeBits(builder);
            const auto offset = layout->getElementOffsetInBits(i);

            elements.emplace_back(
                builder.GetDebug().GetFieldType(
                    field.Name,
                    field.Info.GenDIType(builder),
                    size,
                    offset));
        }

        m_DIType = builder.GetDebug().GetStructType(elements, layout->getSizeInBits());
    }

    return m_DIType;
}

llove::TypePtr llove::StructType::Reflect(Context &context) const
{
    std::vector<Parameter> fields(m_Fields.size());
    for (unsigned i = 0; i < m_Fields.size(); ++i)
    {
        fields.at(i).Name = m_Fields.at(i).Name;
        m_Fields.at(i).Info.Reflect(context, fields.at(i).Info);
    }

    return context.GetStruct(std::move(fields));
}

bool llove::StructType::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    dst.emplace_back(builder.GetI32(ID));
    dst.emplace_back(builder.GetI32(m_Fields.size()));
    for (auto &field : m_Fields)
        if (!field.TypeInfo(builder, dst))
            return false;
    return true;
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
