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

llove::TypePtr llove::Context::TypeUnion(const TypePtr &left, const TypePtr &right)
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

unsigned llove::Context::Difference(const TypePtr &left, const TypePtr &right)
{
    if (left == right)
        return 0u;

    switch (left->GetId())
    {
    case TypeId_Integer:
    {
        const auto left_int = As<IntegerType>(left);
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto right_int = As<IntegerType>(right);
            const auto sign_error = left_int->IsSigned() != right_int->IsSigned() ? 1u : 0u;
            const auto bits_error = left_int->GetBits() != right_int->GetBits() ? 5u : 0u;
            return sign_error + bits_error;
        }
        case TypeId_Float:
        {
            const auto right_flt = As<FloatType>(right);
            const auto bits_error = left_int->GetBits() != right_flt->GetBits() ? 5u : 0u;
            return 5u + bits_error;
        }
        default:
            break;
        }
        break;
    }
    case TypeId_Float:
    {
        const auto left_flt = As<FloatType>(left);
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto right_int = As<IntegerType>(right);
            const auto bits_error = left_flt->GetBits() != right_int->GetBits() ? 5u : 0u;
            return 5u + bits_error;
        }
        case TypeId_Float:
        {
            const auto right_flt = As<FloatType>(right);
            const auto bits_error = left_flt->GetBits() != right_flt->GetBits() ? 5u : 0u;
            return bits_error;
        }
        default:
            break;
        }
        break;
    }
    case TypeId_Pointer:
    {
        const auto left_ptr = As<PointerType>(left);
        switch (right->GetId())
        {
        case TypeId_Integer:
        {
            const auto right_int = As<IntegerType>(right);
            const auto sign_error = false != right_int->IsSigned() ? 1u : 0u;
            const auto bits_error = 64u != right_int->GetBits() ? 5u : 0u;
            return 4u + sign_error + bits_error;
        }
        case TypeId_Pointer:
        {
            const auto right_ptr = As<PointerType>(right);
            const auto opaque_error = left_ptr->IsOpaque() != right_ptr->IsOpaque() ? 2u : 0u;
            const auto left_base = left_ptr->IsOpaque() ? nullptr : left_ptr->GetBase();
            const auto right_base = right_ptr->IsOpaque() ? nullptr : right_ptr->GetBase();
            const auto type_error = left_base != right_base ? 2u : 0u;
            const auto mut_error = left_ptr->IsMutable() != right_ptr->IsMutable() ? 1u : 0u;
            return opaque_error + type_error + mut_error;
        }
        default:
            break;
        }
        break;
    }
    case TypeId_Array:
    {
        const auto left_arr = As<ArrayType>(left);
        switch (right->GetId())
        {
        case TypeId_Pointer:
        {
            const auto right_ptr = As<PointerType>(right);
            const auto opaque_error = false != right_ptr->IsOpaque() ? 2u : 0u;
            const auto right_base = right_ptr->IsOpaque() ? nullptr : right_ptr->GetBase();
            const auto type_error = left_arr->GetBase() != right_base ? 2u : 0u;
            return 6u + opaque_error + type_error;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return 10u;
}

llove::ClassTemplate &llove::Context::PushTemplate(
    std::string name,
    std::vector<std::pair<std::string, TemplateType::Ptr>> parameters)
{
    auto &template_ = m_TemplateTypes.emplace_back();
    for (auto &[fst, snd] : parameters)
        template_.emplace(fst, snd);

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

llove::TypePtr llove::Context::InstantiateTemplateClass(
    Builder &builder,
    std::string name,
    const std::vector<TypePtr> &arguments)
{
    Assert(m_ClassTemplates.contains(name), "undefined class template '{}'", name);

    auto &[
        template_complete,
        template_instantiated,

        template_name,
        template_parameters,
        template_fields,
        template_functions
    ] = m_ClassTemplates.at(name);
    Assert(template_parameters.size() == arguments.size(), "wrong number of type arguments");

    if (!template_complete)
        return std::make_shared<ClassTemplateType>(std::move(name), arguments);

    m_TemplateArguments.clear();

    name = template_name + '.';
    for (unsigned i = 0; i < arguments.size(); ++i)
    {
        m_TemplateArguments.emplace(template_parameters.at(i).first, arguments.at(i));
        name += arguments.at(i)->Mangle();
    }
    auto class_type = GetClass(std::move(name));

    if (template_instantiated)
        return class_type;

    template_instantiated = true;

    std::vector<ClassField> reflection_fields;
    for (auto &field : template_fields)
        field.Reflect(builder, reflection_fields.emplace_back());

    std::vector<ClassFunction> reflection_functions;
    for (auto &function : template_functions)
        function.Reflect(builder, reflection_functions.emplace_back());

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
            function.Implicit,
            function.Mutable,
            function.Name,
            parameters,
            function.VarArg,
            function.Result);
    }
    class_type->SetFunctions(std::move(functions));

    for (auto &[
             expose,
             implicit,
             mutable_,
             name,
             parameters,
             vararg,
             result,
             content
         ] : reflection_functions)
        builder.GenFunction(
            {
                .Implicit = implicit,
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
             implicit,
             mutable_,
             name,
             parameters,
             vararg,
             result,
             content
         ] : reflection_functions)
        builder.GenFunction(
            {
                .Implicit = implicit,
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

llove::TypePtr llove::Context::TemplateArgument(const std::string &name) const
{
    Assert(m_TemplateArguments.contains(name), "undefined template argument '{}'", name);
    return m_TemplateArguments.at(name);
}
