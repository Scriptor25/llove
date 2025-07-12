#include <llove/context.hpp>
#include <llove/parameter.hpp>

llove::TypePtr llove::Context::Get(const std::string &id) const
{
    if (m_TypeMap.contains(id))
        return m_TypeMap.at(id);
    return nullptr;
}

void llove::Context::Set(const std::string &id, TypePtr type)
{
    m_TypeMap[id] = std::move(type);
}

llove::VoidType::Ptr llove::Context::GetVoid()
{
    if (m_Void)
        return m_Void;
    return m_Void = std::make_shared<VoidType>();
}

llove::IntType::Ptr llove::Context::GetInt(bool sign, unsigned bits)
{
    if (auto &type = m_Int[sign][bits])
        return type;
    else
        return type = std::make_shared<IntType>(sign, bits);
}

llove::FltType::Ptr llove::Context::GetFlt(unsigned bits)
{
    if (auto &type = m_Flt[bits])
        return type;
    else
        return type = std::make_shared<FltType>(bits);
}

llove::PtrType::Ptr llove::Context::GetPtr(TypePtr base, bool mutable_)
{
    if (auto &type = m_Ptr[base][mutable_])
        return type;
    else
        return type = std::make_shared<PtrType>(std::move(base), mutable_);
}

llove::ArrayType::Ptr llove::Context::GetArray(TypePtr base, int64_t size)
{
    if (auto &type = m_Array[base][size])
        return type;
    else
        return type = std::make_shared<ArrayType>(std::move(base), size);
}

llove::StructType::Ptr llove::Context::GetStruct(std::vector<Parameter> parameters)
{
    std::vector<Field> fields;
    for (const auto &[info_, name_] : parameters)
        fields.emplace_back(info_);
    const auto hash = GetFieldHash(fields);

    if (auto &type = m_Struct[hash])
        return type;
    else
        return type = std::make_shared<StructType>(std::move(parameters));
}

llove::ClassType::Ptr llove::Context::GetClass(const std::string &name)
{
    if (auto &type = m_Class[name])
        return type;
    else
        return type = std::make_shared<ClassType>(name);
}

llove::FunctionType::Ptr llove::Context::GetFunction(std::vector<Field> parameters, bool vararg, Field result)
{
    const auto parameter_hash = GetFieldHash(parameters);
    const auto result_hash = GetFieldHash({ result });

    if (auto &type = m_Function[parameter_hash][vararg][result_hash][{}])
        return type;
    else
        return type = std::make_shared<FunctionType>(std::move(parameters), vararg, std::move(result), Field{});
}

llove::FunctionType::Ptr llove::Context::GetFunction(
    std::vector<Field> parameters,
    bool vararg,
    Field result,
    Field self)
{
    const auto parameter_hash = GetFieldHash(parameters);
    const auto result_hash = GetFieldHash({ result });
    const auto self_hash = GetFieldHash({ self });

    if (auto &type = m_Function[parameter_hash][vararg][result_hash][self_hash])
        return type;
    else
        return type = std::make_shared<FunctionType>(std::move(parameters), vararg, std::move(result), std::move(self));
}
