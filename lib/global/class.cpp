#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>

llove::ClassGlobal::ClassGlobal(
    Location loc,
    const bool is_export,
    ClassType::Ptr class_type)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_IsOpaque(true),
      m_ClassType(std::move(class_type))
{
}

llove::ClassGlobal::ClassGlobal(
    Location loc,
    const bool is_export,
    ClassType::Ptr class_type,
    ClassType::Ptr base_type,
    std::vector<ClassMember> members,
    std::vector<ClassFunction> functions)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_IsOpaque(false),
      m_ClassType(std::move(class_type)),
      m_BaseType(std::move(base_type)),
      m_Members(std::move(members)),
      m_Functions(std::move(functions))
{
}

std::string llove::ClassGlobal::GetName() const
{
    return m_ClassType->GetName();
}

llove::GlobalPtr llove::ClassGlobal::Reflect(Context &context) const
{
    ClassType::Ptr class_type, base_type;
    Type::Reflect(context, m_ClassType, class_type);
    Type::Reflect(context, m_BaseType, base_type);

    std::vector<ClassMember> members;
    std::vector<ClassFunction> functions;

    for (auto &member : m_Members)
        member.Reflect(context, members.emplace_back());

    for (auto &function : m_Functions)
        function.Reflect(context, functions.emplace_back());

    return std::make_unique<ClassGlobal>(
        m_Loc,
        m_IsExport,
        std::move(class_type),
        std::move(base_type),
        std::move(members),
        std::move(functions));
}

void llove::ClassGlobal::Gen(Builder &builder) const try
{
    if (m_IsOpaque)
        return;

    m_ClassType->SetParentClass(m_BaseType);

    std::vector<ClassMemberReference> class_members;
    for (const auto &[info_, name_] : m_Members)
        class_members.emplace_back(info_, name_);
    m_ClassType->SetMembers(std::move(class_members));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info_, name_] : function.Parameters)
            parameters.push_back(info_);
        class_functions.push_back(
            {
                .IsExport = m_IsExport,
                .IsPublic = function.IsPublic,
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
    m_ClassType->SetFunctions(std::move(class_functions));

    for (auto &function : m_Functions)
    {
        std::vector<Initializer> initializers;
        for (auto &initializer : function.Initializers)
            initializer.Reflect(builder.GetContext(), initializers.emplace_back());

        StatementPtr content;
        if (function.Content)
            function.Content->Reflect(builder.GetContext(), content);

        Function agg;
        agg.Loc = function.Loc;
        agg.IsExport = m_IsExport;
        agg.IsPublic = function.IsPublic;
        agg.IsVirtual = function.IsVirtual;
        agg.IsOverride = function.IsOverride;
        agg.IsImplicit = function.IsImplicit;
        agg.IsMutable = function.IsMutable;
        agg.Class = m_ClassType;
        agg.Name = function.Name;
        agg.Parameters = function.Parameters;
        agg.Variadic = function.Variadic;
        agg.Result = function.Result;
        agg.Initializers = std::move(initializers);
        agg.Content = std::move(content);

        builder.GenFunction(agg, false);
    }
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::TemplateInstancePtr llove::ClassGlobal::GenTemplate(
    Builder *builder,
    Context &context,
    std::string name) const
{
    auto class_type = context.GetClass(std::move(name));

    ClassType::Ptr base_type;
    Type::Reflect(context, m_BaseType, base_type);

    class_type->SetParentClass(base_type);

    std::vector<ClassMemberReference> class_members;
    for (const auto &[info_, name_] : m_Members)
    {
        Field info;
        info_.Reflect(context, info);

        class_members.emplace_back(std::move(info), name_);
    }
    class_type->SetMembers(std::move(class_members));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info_, name_] : function.Parameters)
        {
            Field info;
            info_.Reflect(context, info);

            parameters.push_back(std::move(info));
        }

        class_functions.push_back(
            {
                .IsExport = m_IsExport,
                .IsPublic = function.IsPublic,
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
    class_type->SetFunctions(std::move(class_functions));

    for (auto &function : m_Functions)
    {
        std::vector<Initializer> initializers;
        for (auto &initializer : function.Initializers)
            initializer.Reflect(context, initializers.emplace_back());

        StatementPtr content;
        if (function.Content)
            function.Content->Reflect(context, content);

        Function agg;
        agg.Loc = function.Loc;
        agg.IsExport = m_IsExport;
        agg.IsPublic = function.IsPublic;
        agg.IsVirtual = function.IsVirtual;
        agg.IsOverride = function.IsOverride;
        agg.IsImplicit = function.IsImplicit;
        agg.IsMutable = function.IsMutable;
        agg.Class = class_type;
        agg.Name = function.Name;
        agg.Parameters = function.Parameters;
        agg.Variadic = function.Variadic;
        agg.Result = function.Result;
        agg.Initializers = std::move(initializers);
        agg.Content = std::move(content);

        // TODO
        // builder->GenFunction(agg, false);
    }

    return std::make_unique<TypeTemplateInstance>(std::move(class_type));
}

llove::Import llove::ClassGlobal::GenImport(
    Context &context,
    Builder & /* builder */,
    const std::string &as,
    const ImportSymbols &symbols) const
{
    if (!m_IsExport)
        return {};

    auto &name = m_ClassType->GetName();

    if (as.empty() && !symbols.empty() && !symbols.contains(name))
        return {};

    context.GetParent()->Set(m_ClassType->Mangle(), m_ClassType);

    if (const auto it = symbols.find(name); it != symbols.end())
        context.GetParent()->SetNamed(it->second, m_ClassType);
    else
        context.GetParent()->SetNamed(name, m_ClassType);

    if (m_IsOpaque)
        return { name, nullptr };

    m_ClassType->SetParentClass(m_BaseType);

    std::vector<ClassMemberReference> class_members;
    for (const auto &[info_, name_] : m_Members)
        class_members.emplace_back(info_, name_);
    m_ClassType->SetMembers(std::move(class_members));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info_, name_] : function.Parameters)
            parameters.push_back(info_);
        class_functions.push_back(
            {
                .IsExport = m_IsExport,
                .IsPublic = function.IsPublic,
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
    m_ClassType->SetFunctions(std::move(class_functions));

    return { name, nullptr };
}

std::ostream &llove::ClassGlobal::Print(std::ostream &stream) const
{
    stream << "class " << m_ClassType->GetName();
    if (m_IsOpaque)
        return stream << ";";

    if (m_BaseType)
        stream << " : " << m_BaseType->GetName();

    const auto cur = std::string(PrintDepth += 2, ' ');

    stream << " {" << std::endl;
    for (auto &function : m_Functions)
        stream << cur << function << std::endl;
    if (!m_Functions.empty() && !m_Members.empty())
        stream << std::endl;
    for (auto &member : m_Members)
        stream << cur << member << std::endl;
    return stream << std::string(PrintDepth -= 2, ' ') << '}';
}
