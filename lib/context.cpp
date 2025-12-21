#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <ranges>

llove::Context::Context(Context* parent)
    : m_Parent(parent)
{
}

llove::Context* llove::Context::GetParent() const
{
    return m_Parent;
}

llove::TypePtr llove::Context::GetNamed(const std::string& id) const
{
    for (auto& frame : std::ranges::reverse_view(m_TemplateParameterStack))
        for (auto& parameter : frame)
            if (parameter.first == id)
                return parameter.second;

    if (m_Named.contains(id))
        return m_Named.at(id);

    return nullptr;
}

void llove::Context::SetNamed(
    const std::string& id,
    TypePtr type)
{
    m_Named.emplace(id, type);
}

void llove::Context::Set(
    std::string hash,
    TypePtr type)
{
    m_Types.emplace(hash, type);
}

llove::VoidType::Ptr llove::Context::GetVoid()
{
    return GetOrCreate<VoidType>();
}

llove::VariadicType::Ptr llove::Context::GetVariadic()
{
    return GetOrCreate<VariadicType>();
}

llove::IntegerType::Ptr llove::Context::GetInteger(
    bool is_signed,
    unsigned bits)
{
    return GetOrCreate<IntegerType>(is_signed, bits);
}

llove::FloatType::Ptr llove::Context::GetFloat(unsigned bits)
{
    return GetOrCreate<FloatType>(bits);
}

llove::PointerType::Ptr llove::Context::GetPointer(const bool is_mutable)
{
    return GetOrCreate<PointerType>(is_mutable);
}

llove::PointerType::Ptr llove::Context::GetPointer(
    TypePtr base,
    bool is_mutable)
{
    return GetOrCreate<PointerType>(std::move(base), is_mutable);
}

llove::ArrayType::Ptr llove::Context::GetArray(
    TypePtr base,
    unsigned size)
{
    return GetOrCreate<ArrayType>(std::move(base), size);
}

llove::StructType::Ptr llove::Context::GetStruct(std::vector<Parameter> fields)
{
    return GetOrCreate<StructType>(std::move(fields));
}

llove::TupleType::Ptr llove::Context::GetTuple(std::vector<Field> fields)
{
    return GetOrCreate<TupleType>(std::move(fields));
}

llove::RangeType::Ptr llove::Context::GetRange(TypePtr entry)
{
    return GetOrCreate<RangeType>(std::move(entry));
}

llove::ClassType::Ptr llove::Context::GetClass(std::string name)
{
    return GetOrCreate<ClassType>(std::move(name));
}

llove::FunctionType::Ptr llove::Context::GetFunction(
    Field result,
    std::vector<Field> parameters,
    bool variadic,
    std::optional<Field> self)
{
    return GetOrCreate<FunctionType>(std::move(result), std::move(parameters), variadic, std::move(self));
}

llove::InstanceType::Ptr llove::Context::GetInstance(std::string name)
{
    std::vector<TypePtr> arguments;
    for (auto& frame = m_TemplateParameterStack.back(); auto& parameter : frame)
        arguments.emplace_back(parameter.second);

    return GetInstance(std::move(name), std::move(arguments));
}

llove::InstanceType::Ptr llove::Context::GetInstance(
    std::string name,
    std::vector<TypePtr> arguments)
{
    return GetOrCreate<InstanceType>(std::move(name), std::move(arguments));
}

llove::IntegerType::Ptr llove::Context::GetBoolean()
{
    return GetOrCreate<IntegerType>(false, 1);
}

llove::TypePtr llove::Context::TypeUnion(
    TypePtr left,
    TypePtr right)
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
            const auto sign = As<IntegerType>(left)->IsSigned()
                           || As<IntegerType>(right)->IsSigned();
            const auto bits = std::max(
                As<IntegerType>(left)->GetBits(),
                As<IntegerType>(right)->GetBits());
            return GetInteger(sign, bits);
        }
        case TypeId_Float:
        {
            const auto bits = std::max(
                As<IntegerType>(left)->GetBits(),
                As<FloatType>(right)->GetBits());
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
            const auto bits = std::max(
                As<FloatType>(left)->GetBits(),
                As<IntegerType>(right)->GetBits());
            return GetFloat(bits);
        }
        case TypeId_Float:
        {
            const auto bits = std::max(
                As<FloatType>(left)->GetBits(),
                As<FloatType>(right)->GetBits());
            return GetFloat(bits);
        }
        default:
            break;
        }
        break;

    case TypeId_Pointer:
        switch (right->GetId())
        {
        case TypeId_Pointer:
        {
            const auto left_pointer = As<PointerType>(left);
            const auto right_pointer = As<PointerType>(right);

            if (!left_pointer->IsOpaque() && !right_pointer->IsOpaque()
                && left_pointer->GetBase() != right_pointer->GetBase())
                break;

            const auto base = !left_pointer->IsOpaque() ? left_pointer->GetBase()
                            : !right_pointer->IsOpaque() ? right_pointer->GetBase()
                                                         : nullptr;
            const auto is_mutable = left_pointer->IsMutable() && right_pointer->IsMutable();

            if (base)
                return GetPointer(base, is_mutable);

            return GetPointer(is_mutable);
        }
        default:
            break;
        }
        break;

    default:
        break;
    }

    Error("illegal type unionization of {} and {}", left, right);
}

void llove::Context::PushTemplate(const std::vector<TemplateParameter>& parameters)
{
    auto& frame = m_TemplateParameterStack.emplace_back();
    for (auto& entry : parameters)
        frame.emplace_back(entry);
}

void llove::Context::PopTemplate()
{
    m_TemplateParameterStack.pop_back();
}

void llove::Context::CreateTemplate(
    const std::string& name,
    TemplateInstancePtr instance)
{
    Assert(!m_TemplateParameterStack.empty(), "no current template");

    m_Templates[name] = {
        name,
        m_TemplateParameterStack.back(),
        nullptr,
        std::move(instance),
    };
}

void llove::Context::CreateTemplate(
    const std::string& name,
    std::vector<TemplateParameter> parameters,
    GlobalPtr content)
{
    m_Templates[name] = {
        name,
        std::move(parameters),
        std::move(content),
        nullptr,
    };
}

llove::TemplateInstance* llove::Context::InstantiateUniqueTemplate(
    Builder* builder,
    const std::string& name,
    const std::vector<TypePtr>& type_arguments)
{
    auto index = name + '<';
    for (auto it = type_arguments.begin(); it != type_arguments.end(); ++it)
    {
        if (it != type_arguments.begin())
            index += ',';
        index += (*it)->Mangle();
    }
    index += '>';

    auto contains = m_Instances.contains(index);
    if (contains)
        if (auto ptr = m_Instances.at(index).get())
            return ptr;

    Assert(m_Templates.contains(name), "undefined template name {}", name);

    auto& temp = m_Templates.at(name);

    Assert(type_arguments.size() == temp.Parameters.size(), "invalid number of type argument");

    auto& ref = m_Instances[index];

    auto& frame = m_TemplateArgumentStack.emplace_back();
    for (auto i = 0u; i < type_arguments.size(); ++i)
        frame.emplace(temp.Parameters.at(i).first, type_arguments.at(i));

    TemplateInstance* ptr;
    if (!contains && temp.Content)
    {
        ref = temp.Content->GenTemplate(builder, *this, std::move(index));
        ptr = ref.get();
    }
    else
    {
        ptr = temp.Default.get();
    }

    m_TemplateArgumentStack.pop_back();
    return ptr;
}

bool llove::Context::IsInstantiating() const
{
    return !m_TemplateArgumentStack.empty();
}

llove::TypePtr llove::Context::GetTemplateArgument(const std::string& name) const
{
    Assert(!m_TemplateArgumentStack.empty(), "not a template");
    auto& frame = m_TemplateArgumentStack.back();
    Assert(frame.contains(name), "undefined template parameter '{}'", name);
    return frame.at(name);
}
