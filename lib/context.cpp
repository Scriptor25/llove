#include <llove/context.hpp>
#include <llove/error.hpp>

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

llove::IntegerType::Ptr llove::Context::GetInteger(bool sign, unsigned bits)
{
    if (auto &type = m_Integer[sign][bits])
        return type;
    else
        return type = std::make_shared<IntegerType>(sign, bits);
}

llove::FloatType::Ptr llove::Context::GetFloat(unsigned bits)
{
    if (auto &type = m_Float[bits])
        return type;
    else
        return type = std::make_shared<FloatType>(bits);
}

llove::PointerType::Ptr llove::Context::GetPointer(const bool mutable_)
{
    return GetPointer(nullptr, mutable_);
}

llove::PointerType::Ptr llove::Context::GetPointer(TypePtr base, bool mutable_)
{
    if (auto &type = m_Pointer[base][mutable_])
        return type;
    else
        return type = std::make_shared<PointerType>(std::move(base), mutable_);
}

llove::ArrayType::Ptr llove::Context::GetArray(TypePtr base, unsigned size)
{
    if (auto &type = m_Array[base][size])
        return type;
    else
        return type = std::make_shared<ArrayType>(std::move(base), size);
}

llove::StructType::Ptr llove::Context::GetStruct(std::vector<Parameter> fields)
{
    std::vector<Field> struct_fields;
    for (const auto &[info_, name_] : fields)
        struct_fields.emplace_back(info_);
    const auto hash = GetFieldHash(struct_fields);

    if (auto &type = m_Struct[hash])
        return type;
    else
        return type = std::make_shared<StructType>(std::move(fields));
}

llove::RangeType::Ptr llove::Context::GetRange(TypePtr entry)
{
    if (auto &type = m_Range[entry])
        return type;
    else
        return type = std::make_shared<RangeType>(std::move(entry));
}

llove::ClassType::Ptr llove::Context::GetClass(std::string name)
{
    if (auto &type = m_Class[name])
        return type;
    else
        return type = std::make_shared<ClassType>(std::move(name));
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

bool llove::Context::HasClass(const std::string &name) const
{
    return m_Class.contains(name);
}

llove::TypePtr llove::Context::DetermineHigherOrder(const TypePtr &left, const TypePtr &right)
{
    if (left == right)
        return left;

    switch (left->GetId())
    {
    case TypeId_Integer:
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto sign = As<IntegerType>(left)->IsSigned() || As<IntegerType>(right)->IsSigned();
            const auto bits = std::max(As<IntegerType>(left)->GetBits(), As<IntegerType>(right)->GetBits());
            return GetInteger(sign, bits);
        }
        case TypeId_Float:
        {
            const auto bits = std::max(As<IntegerType>(left)->GetBits(), As<FloatType>(right)->GetBits());
            return GetFloat(bits);
        }
        default:
            break;
        }
        break;

    case TypeId_Float:
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto bits = std::max(As<FloatType>(left)->GetBits(), As<IntegerType>(right)->GetBits());
            return GetFloat(bits);
        }
        case TypeId_Float:
        {
            const auto bits = std::max(As<FloatType>(left)->GetBits(), As<FloatType>(right)->GetBits());
            return GetFloat(bits);
        }
        default:
            break;
        }
        break;

    default:
        break;
    }

    Error("cannot determine higher order of types {} and {}", left, right);
}
