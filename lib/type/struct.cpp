#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::StructType::StructType(std::vector<ClassFieldReference> fields)
    : m_Fields(std::move(fields))
{
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

llvm::StructType *llove::StructType::Gen(Builder &builder) const
{
    std::vector<llvm::Type *> fields;
    for (auto &[info_, name_] : m_Fields)
        fields.emplace_back(info_.GenType(builder));

    // TODO: packed struct
    return builder.GetStructType(fields, true);
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
