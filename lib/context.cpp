#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::Context::Context(Context *parent)
    : m_Parent(parent)
{
}

llove::Context *llove::Context::GetParent() const
{
    return m_Parent;
}

llove::TypePtr llove::Context::GetNamed(const std::string &id) const
{
    for (auto &template_ : std::ranges::reverse_view(m_TemplateTypes))
        if (template_.contains(id))
            return template_.at(id);

    if (m_Named.contains(id))
        return m_Named.at(id);

    return nullptr;
}

void llove::Context::SetNamed(const std::string &id, TypePtr type)
{
    m_Named.emplace(id, type);
}

void llove::Context::Set(std::string hash, TypePtr type)
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

llove::IntegerType::Ptr llove::Context::GetInteger(bool is_signed, unsigned bits)
{
    return GetOrCreate<IntegerType>(is_signed, bits);
}

llove::FloatType::Ptr llove::Context::GetFloat(unsigned bits)
{
    return GetOrCreate<FloatType>(bits);
}

llove::PointerType::Ptr llove::Context::GetPointer(const bool mutable_)
{
    return GetOrCreate<PointerType>(mutable_);
}

llove::PointerType::Ptr llove::Context::GetPointer(TypePtr base, bool mutable_)
{
    return GetOrCreate<PointerType>(std::move(base), mutable_);
}

llove::ArrayType::Ptr llove::Context::GetArray(TypePtr base, unsigned size)
{
    return GetOrCreate<ArrayType>(std::move(base), size);
}

llove::StructType::Ptr llove::Context::GetStruct(std::vector<Parameter> fields)
{
    return GetOrCreate<StructType>(std::move(fields));
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
    std::vector<Field> parameters,
    bool variadic,
    Field result,
    std::optional<Field> self)
{
    return GetOrCreate<FunctionType>(std::move(parameters), variadic, std::move(result), std::move(self));
}

llove::IntegerType::Ptr llove::Context::GetBoolean()
{
    return GetOrCreate<IntegerType>(false, 1);
}

llove::TypePtr llove::Context::TypeUnion(TypePtr left, TypePtr right)
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

    Error("illegal type unionization of {} and {}", left, right);
}

llove::ClassTemplate &llove::Context::PushTemplate(
    std::string name,
    std::vector<std::pair<std::string, TemplateType::Ptr>> parameters)
{
    auto &template_ = m_TemplateTypes.emplace_back();
    for (auto &[key, type] : parameters)
        template_.emplace(key, std::move(type));

    auto &ref = m_ClassTemplates[name];

    m_CurrentTemplate = &ref;

    return ref = {
               .Name = std::move(name),
               .Parameters = std::move(parameters),
           };
}

void llove::Context::PopTemplate()
{
    m_CurrentTemplate->Complete = true;

    m_CurrentTemplate = nullptr;
    m_TemplateTypes.pop_back();
}

void llove::Context::EmplaceTemplate(
    std::string name,
    std::vector<std::pair<std::string, TemplateType::Ptr>> parameters)
{
    auto &ref = m_ClassTemplates[name];

    ref = {
        .Name = std::move(name),
        .Parameters = std::move(parameters),
    };
}

llove::TypePtr llove::Context::InstantiateTemplateClass(std::string name, std::vector<TypePtr> arguments)
{
    Assert(m_ClassTemplates.contains(name), "undefined class template '{}'", name);

    auto &template_ = m_ClassTemplates.at(name);
    Assert(template_.Parameters.size() == arguments.size(), "wrong number of type arguments");

    if (!template_.Complete)
        return std::make_shared<ClassTemplateType>(std::move(name), std::move(arguments));

    m_TemplateArguments.clear();

    name = template_.Name + '<';
    for (unsigned i = 0; i < arguments.size(); ++i)
    {
        if (i)
            name += ", ";
        m_TemplateArguments.emplace(template_.Parameters.at(i).first, arguments.at(i));
        name += arguments.at(i)->Mangle();
    }
    name += '>';
    const auto class_type = GetClass(std::move(name));

    if (template_.Instantiated)
        return class_type;

    template_.Instantiated = true;

    std::vector<ClassField> reflection_fields;
    for (auto &field : template_.Fields)
        field.Reflect(*this, reflection_fields.emplace_back());

    std::vector<ClassFunction> reflection_functions;
    for (auto &function : template_.Functions)
        function.Reflect(*this, reflection_functions.emplace_back());

    std::vector<ClassFieldReference> fields;
    for (auto &field : reflection_fields)
        fields.emplace_back(field.Info, field.Name);
    class_type->SetMembers(std::move(fields));

    std::vector<ClassFunctionReference> functions;
    for (const auto &function : reflection_functions)
    {
        std::vector<Field> parameters;
        for (auto &parameter : function.Parameters)
            parameters.emplace_back(parameter.Info);
        functions.emplace_back(
            function.Expose,
            function.Implicit,
            function.Mutable,
            function.Name,
            parameters,
            function.Variadic.first,
            function.Result);
    }
    class_type->SetFunctions(std::move(functions));

    m_Reflections[class_type] = std::move(reflection_functions);

    return class_type;
}

void llove::Context::InstantiateReflections(Builder &builder)
{
    for (auto &[class_type, functions] : m_Reflections)
    {
        for (const auto &function : functions)
            builder.GenFunction(
                {
                    .Loc = function.Loc,
                    .Interface = false,
                    .Implicit = function.Implicit,
                    .Class = class_type,
                    .Mutable = function.Mutable,
                    .Expose = function.Expose,
                    .Name = function.Name,
                    .Parameters = function.Parameters,
                    .Variadic = function.Variadic,
                    .Result = function.Result,
                    .Content = nullptr,
                }
            );

        for (const auto &function : functions)
            builder.GenFunction(
                {
                    .Loc = function.Loc,
                    .Interface = false,
                    .Implicit = function.Implicit,
                    .Class = class_type,
                    .Mutable = function.Mutable,
                    .Expose = function.Expose,
                    .Name = function.Name,
                    .Parameters = function.Parameters,
                    .Variadic = function.Variadic,
                    .Result = function.Result,
                    .Content = function.Content.get(),
                }
            );
    }

    m_Reflections.clear();
}

llove::TypePtr llove::Context::TemplateArgument(const std::string &name) const
{
    Assert(m_TemplateArguments.contains(name), "undefined template argument '{}'", name);
    return m_TemplateArguments.at(name);
}
