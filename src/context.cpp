#include <llove/context.hpp>
#include <llove/parameter.hpp>

llove::TypePtr llove::Context::Get(const std::string &id) const
{
    if (m_TypeMap.contains(id))
        return m_TypeMap.at(id);
    return nullptr;
}

void llove::Context::Set(const std::string &id, const TypePtr &type)
{
    m_TypeMap[id] = type;
}

llove::TypePtr llove::Context::GetVoid()
{
    if (m_Void)
        return m_Void;
    return m_Void = std::make_shared<VoidType>();
}

llove::TypePtr llove::Context::GetInt(bool sign, unsigned bits)
{
    if (auto &type = m_Int[sign][bits])
        return type;
    else
        return type = std::make_shared<IntType>(sign, bits);
}

llove::TypePtr llove::Context::GetFlt(unsigned bits)
{
    if (auto &type = m_Flt[bits])
        return type;
    else
        return type = std::make_shared<FltType>(bits);
}

llove::TypePtr llove::Context::GetArray(const TypePtr &base, int64_t size)
{
    if (auto &type = m_Array[base][size])
        return type;
    else
        return type = std::make_shared<ArrayType>(base, size);
}

llove::TypePtr llove::Context::GetStruct(const std::vector<Parameter> &parameters)
{
    std::vector<Field> fields;
    for (const auto &[info_, name_] : parameters)
        fields.emplace_back(info_);
    const auto hash = GetFieldHash(fields);

    if (auto &type = m_Struct[hash])
        return type;
    else
        return type = std::make_shared<StructType>(parameters);
}
