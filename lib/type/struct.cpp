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
    for (const auto &[_, field_name] : m_Fields)
        if (field_name == name)
            return true;
    return false;
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

unsigned llove::StructType::Size(Builder &builder) const
{
    auto size = 0u;
    for (auto &[info, name] : m_Fields)
        size += info.Size(builder);
    return size;
}

llvm::StructType *llove::StructType::Gen(Builder &builder) const
{
    std::vector<llvm::Type *> fields;
    for (auto &[info_, name_] : m_Fields)
        fields.emplace_back(info_.GenType(builder));

    // TODO: packed struct
    return builder.GetStructType(fields, true);
}

llove::TypePtr llove::StructType::Reflect(Context &types) const
{
    std::vector<Parameter> fields(m_Fields.size());
    for (unsigned i = 0; i < m_Fields.size(); ++i)
    {
        fields.at(i).Name = m_Fields.at(i).Name;
        m_Fields.at(i).Info.Reflect(types, fields.at(i).Info);
    }
    return types.GetStruct(std::move(fields));
}

std::string llove::StructType::Mangle() const
{
    std::string fields;
    for (auto &[info_, name_] : m_Fields)
        fields += info_.Mangle();
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
