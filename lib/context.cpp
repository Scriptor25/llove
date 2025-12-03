#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
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
    for (auto& template_ : std::ranges::reverse_view(m_TemplateTypes))
        if (template_.contains(id))
            return template_.at(id);

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

llove::ClassTemplate& llove::Context::PushClassTemplate(
    const bool is_export,
    std::string name,
    std::vector<std::pair<
        std::string,
        TemplateType::Ptr>> type_parameters,
    const bool is_imported)
{
    if (!is_imported)
    {
        auto& template_frame = m_TemplateTypes.emplace_back();
        for (auto& [key, type] : type_parameters)
            template_frame.emplace(key, type);
    }

    if (is_export && m_Parent)
    {
        auto& ref = m_Parent->PushClassTemplate(false, std::move(name), std::move(type_parameters), true);
        ref.IsImported = true;

        if (!is_imported)
            m_CurrentClassTemplate = &ref;

        return ref;
    }

    auto& ref = m_ClassTemplates[name];

    if (!is_imported)
        m_CurrentClassTemplate = &ref;

    ref.Name = std::move(name);
    ref.TypeParameters = std::move(type_parameters);

    return ref;
}

void llove::Context::PopClassTemplate()
{
    m_CurrentClassTemplate->Complete = true;
    m_CurrentClassTemplate = nullptr;

    m_TemplateTypes.pop_back();
}

llove::ClassTemplate& llove::Context::EmplaceClassTemplate(
    const bool is_export,
    std::string name,
    std::vector<std::pair<
        std::string,
        TemplateType::Ptr>> type_parameters)
{
    if (is_export && m_Parent)
    {
        auto& ref = m_Parent->EmplaceClassTemplate(false, std::move(name), std::move(type_parameters));
        ref.IsImported = true;

        return ref;
    }

    auto& ref = m_ClassTemplates[name];

    ref.Name = std::move(name);
    ref.TypeParameters = std::move(type_parameters);

    return ref;
}

llove::DefinitionTemplate& llove::Context::PushDefinitionTemplate(
    const bool is_export,
    const bool is_implicit,
    Location loc,
    std::string name,
    std::vector<std::pair<
        std::string,
        TemplateType::Ptr>> type_parameters,
    const bool is_imported)
{
    if (!is_imported)
    {
        auto& template_frame = m_TemplateTypes.emplace_back();
        for (auto& [key, type] : type_parameters)
            template_frame.emplace(key, type);
    }

    if (is_export && m_Parent)
    {
        auto& ref = m_Parent->PushDefinitionTemplate(false, is_implicit, std::move(loc), std::move(name), std::move(type_parameters), true);
        ref.IsImported = true;

        return ref;
    }

    auto& ref = m_DefinitionTemplates[name];

    ref.Loc = std::move(loc);
    ref.IsImplicit = is_implicit;
    ref.Name = std::move(name);
    ref.TypeParameters = std::move(type_parameters);

    return ref;
}

void llove::Context::PopDefinitionTemplate()
{
    m_TemplateTypes.pop_back();
}

llove::TypePtr llove::Context::InstantiateClass(
    std::string name,
    std::vector<TypePtr> type_arguments,
    const bool is_imported)
{
    if (!m_ClassTemplates.contains(name) && m_Parent)
    {
        return m_Parent->InstantiateClass(std::move(name), std::move(type_arguments), true);
    }

    Assert(m_ClassTemplates.contains(name), "undefined class template '{}'", name);

    const auto& class_template = m_ClassTemplates.at(name);
    Assert(!is_imported || class_template.IsImported, "template is not imported, cannot be accessed from child context");
    Assert(class_template.TypeParameters.size() == type_arguments.size(), "wrong number of type arguments");

    auto complete = class_template.Complete;
    for (auto it = type_arguments.begin(); it != type_arguments.end() && complete; ++it)
    {
        complete = !(*it)->IsTemplate();
    }

    auto template_class = GetOrCreate<TemplateClassType>(name, type_arguments);
    if (!complete)
    {
        return template_class;
    }

    name = class_template.Name + '<';
    for (unsigned i = 0; i < type_arguments.size(); ++i)
    {
        if (i)
        {
            name += ", ";
        }
        name += type_arguments.at(i)->Mangle();
    }
    name += '>';

    auto class_type = GetClass(std::move(name));
    if (template_class->IsInstantiated())
    {
        return class_type;
    }

    std::map<std::string, TypePtr> frame;
    for (unsigned i = 0; i < type_arguments.size(); ++i)
    {
        frame.emplace(class_template.TypeParameters.at(i).first, type_arguments.at(i));
    }

    m_TemplateStack.push_back(&frame);
    template_class->Instantiate();

    std::vector<ClassMember> reflection_members;
    for (const auto& member : class_template.Members)
    {
        member.Reflect(*this, reflection_members.emplace_back());
    }

    std::vector<ClassFunction> reflection_functions;
    for (const auto& function : class_template.Functions)
    {
        function.Reflect(*this, reflection_functions.emplace_back());
    }

    std::vector<ClassMemberReference> members;
    for (const auto& member : reflection_members)
    {
        members.emplace_back(member.Info, member.Name);
    }
    class_type->SetMembers(std::move(members));

    std::vector<ClassFunctionReference> functions;
    for (const auto& function : reflection_functions)
    {
        std::vector<Field> parameters;
        for (const auto& parameter : function.Parameters)
        {
            parameters.emplace_back(parameter.Info);
        }

        functions.emplace_back(
            ClassFunctionReference{
                .IsExport = false,
                .IsExposed = function.IsExposed,
                .IsVirtual = function.IsVirtual,
                .IsOverride = function.IsOverride,
                .IsImplicit = function.IsImplicit,
                .IsMutable = function.IsMutable,
                .Name = function.Name,
                .Parameters = std::move(parameters),
                .HasVariadic = function.Variadic.first,
                .Result = function.Result,
            });
    }
    class_type->SetFunctions(std::move(functions));

    m_TemplateStack.pop_back();

    auto& ref = m_ClassReflections.emplace_back();

    ref.Frame = std::move(frame);
    ref.Class = class_type;
    ref.Functions = std::move(reflection_functions);

    return class_type;
}

llove::FunctionReference& llove::Context::InstantiateDefinition(
    Builder& builder,
    std::string name,
    std::vector<TypePtr> type_arguments,
    const bool is_imported)
{
    if (!m_DefinitionTemplates.contains(name) && m_Parent)
    {
        return m_Parent->InstantiateDefinition(builder, std::move(name), std::move(type_arguments), true);
    }

    Assert(m_DefinitionTemplates.contains(name), "undefined definition template '{}'", name);

    const auto& definition_template = m_DefinitionTemplates.at(name);
    Assert(!is_imported || definition_template.IsImported, "template is not imported, cannot be accessed from child context");
    Assert(definition_template.TypeParameters.size() == type_arguments.size(), "wrong number of type arguments");

    name = definition_template.Name + '<';
    for (unsigned i = 0; i < type_arguments.size(); ++i)
    {
        if (i)
        {
            name += ", ";
        }
        name += type_arguments.at(i)->Mangle();
    }
    name += '>';

    if (m_DefinitionInstances.contains(name))
    {
        return m_DefinitionInstances.at(name);
    }

    std::map<std::string, TypePtr> frame;
    for (unsigned i = 0; i < type_arguments.size(); ++i)
    {
        frame.emplace(
            definition_template.TypeParameters.at(i).first,
            type_arguments.at(i));
    }

    m_TemplateStack.push_back(&frame);

    std::vector<Parameter> parameters;
    for (auto& parameter : definition_template.Parameters)
    {
        auto& reflection = parameters.emplace_back();
        reflection.Name = parameter.Name;
        parameter.Info.Reflect(*this, reflection.Info);
    }

    Field result;
    definition_template.Result.Reflect(*this, result);

    StatementPtr content;
    if (definition_template.Content)
    {
        definition_template.Content->Reflect(*this, content);
    }

    m_TemplateStack.pop_back();

    {
        Function agg;
        agg.Loc = definition_template.Loc;
        agg.IsImplicit = definition_template.IsImplicit;
        agg.Name = definition_template.Name;
        agg.Parameters = parameters;
        agg.Variadic = definition_template.Variadic;
        agg.Result = result;
        agg.Content = std::move(content);

        m_DefinitionReflections.emplace_back(std::move(frame), std::move(agg));
    }

    Function agg;
    agg.Loc = definition_template.Loc;
    agg.IsImplicit = definition_template.IsImplicit;
    agg.Name = definition_template.Name;
    agg.Parameters = parameters;
    agg.Variadic = definition_template.Variadic;
    agg.Result = result;

    return m_DefinitionInstances[name] = builder.GenFunction(agg, true);
}

void llove::Context::InstantiateReflections(Builder& builder)
{
    for (auto& [frame, class_type, functions] : m_ClassReflections)
    {
        m_TemplateStack.push_back(&frame);
        for (auto& function : functions)
        {
            Function agg;
            agg.Loc = std::move(function.Loc);
            agg.IsExposed = function.IsExposed;
            agg.IsVirtual = function.IsVirtual;
            agg.IsOverride = function.IsOverride;
            agg.IsImplicit = function.IsImplicit;
            agg.IsMutable = function.IsMutable;
            agg.Class = class_type;
            agg.Name = std::move(function.Name);
            agg.Parameters = std::move(function.Parameters);
            agg.Variadic = std::move(function.Variadic);
            agg.Result = std::move(function.Result);
            agg.Initializers = std::move(function.Initializers);
            agg.Content = std::move(function.Content);

            builder.GenFunction(agg);
        }
        m_TemplateStack.pop_back();
    }

    m_ClassReflections.clear();

    for (const auto& [frame, function] : m_DefinitionReflections)
    {
        m_TemplateStack.push_back(&frame);
        builder.GenFunction(function, true);
        m_TemplateStack.pop_back();
    }

    m_DefinitionReflections.clear();
}

llove::TypePtr llove::Context::TemplateArgument(const std::string& name) const
{
    Assert(!m_TemplateStack.empty(), "not a template");
    const auto frame = m_TemplateStack.back();
    Assert(frame->contains(name), "undefined template argument '{}'", name);
    return frame->at(name);
}
