#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>

llove::TypePtr llove::Context::Get(const std::string &id) const
{
    for (auto &template_ : std::ranges::reverse_view(m_TemplateTypes))
        if (template_.contains(id))
            return template_.at(id);
    if (m_Named.contains(id))
        return m_Named.at(id);
    return nullptr;
}

void llove::Context::Set(const std::string &id, TypePtr type)
{
    m_Named[id] = std::move(type);
}

llove::VoidType::Ptr llove::Context::GetVoid()
{
    return GetOrCreate<VoidType>();
}

llove::IntegerType::Ptr llove::Context::GetInteger(bool sign, unsigned bits)
{
    return GetOrCreate<IntegerType>(sign, bits);
}

llove::FloatType::Ptr llove::Context::GetFloat(unsigned bits)
{
    return GetOrCreate<FloatType>(bits);
}

llove::PointerType::Ptr llove::Context::GetPointer(const bool mutable_)
{
    return GetPointer(nullptr, mutable_);
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

llove::FunctionType::Ptr llove::Context::GetFunction(std::vector<Field> parameters, bool vararg, Field result)
{
    return GetOrCreate<FunctionType>(std::move(parameters), vararg, std::move(result), Field{});
}

llove::FunctionType::Ptr llove::Context::GetFunction(
    std::vector<Field> parameters,
    bool vararg,
    Field result,
    Field self)
{
    return GetOrCreate<FunctionType>(std::move(parameters), vararg, std::move(result), std::move(self));
}

llove::TypePtr llove::Context::GetMax(const TypePtr &left, const TypePtr &right)
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

llove::ClassTemplate &llove::Context::PushTemplate(
    std::string name,
    std::vector<std::pair<std::string, TemplateType::Ptr>> parameters)
{
    auto &template_ = m_TemplateTypes.emplace_back();
    for (auto &[fst, snd] : parameters)
        template_.emplace(fst, snd);

    auto &ref = m_ClassTemplates[name];

    return ref = {
               .Name = std::move(name),
               .Parameters = std::move(parameters),
           };
}

void llove::Context::PopTemplate()
{
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

llove::ClassType::Ptr llove::Context::InstantiateTemplateClass(
    Builder &builder,
    std::string name,
    const std::vector<TypePtr> &arguments)
{
    Assert(m_ClassTemplates.contains(name), "undefined class template '{}'", name);

    const auto &[
        template_name,
        template_parameters,
        template_fields,
        template_functions
    ] = m_ClassTemplates.at(name);
    Assert(template_parameters.size() == arguments.size(), "wrong number of type arguments");

    std::vector<ClassField> reflection_fields;
    for (auto &field : template_fields)
        field.Reflect(*this, reflection_fields.emplace_back());

    std::vector<ClassFunction> reflection_functions;
    for (auto &function : template_functions)
        function.Reflect(*this, reflection_functions.emplace_back());

    name = template_name + '.';
    for (unsigned i = 0; i < arguments.size(); ++i)
        name += arguments.at(i)->Mangle();
    auto class_type = GetClass(std::move(name));

    std::vector<ClassFieldReference> fields;
    for (auto &field : reflection_fields)
        fields.emplace_back(field.Info, field.Name);
    class_type->SetFields(builder, std::move(fields));

    std::vector<ClassFunctionReference> functions;
    for (auto &function : reflection_functions)
    {
        std::vector<Field> parameters;
        for (auto &[info, _] : function.Parameters)
            parameters.emplace_back(info);
        functions.emplace_back(
            function.Expose,
            function.Mutable,
            function.Name,
            parameters,
            function.VarArg,
            function.Result);
    }
    class_type->SetFunctions(std::move(functions));

    for (auto &[
             expose,
             mutable_,
             name,
             parameters,
             vararg,
             result,
             content
         ] : reflection_functions)
        builder.GenFunction(
            {
                .Class = class_type,
                .Mutable = mutable_,
                .Expose = expose,
                .Name = name,
                .Parameters = parameters,
                .VarArg = vararg,
                .Result = result,
            }
        );

    for (auto &[
             expose,
             mutable_,
             name,
             parameters,
             vararg,
             result,
             content
         ] : reflection_functions)
        builder.GenFunction(
            {
                .Class = class_type,
                .Mutable = mutable_,
                .Expose = expose,
                .Name = name,
                .Parameters = parameters,
                .VarArg = vararg,
                .Result = result,
                .Content = content.get(),
            }
        );

    return class_type;
}
