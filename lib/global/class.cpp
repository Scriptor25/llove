#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::ClassGlobal::ClassGlobal(Location loc, const bool export_, ClassType::Ptr type)
    : Global(std::move(loc)),
      m_Export(export_),
      m_Opaque(true),
      m_Type(std::move(type))
{
}

llove::ClassGlobal::ClassGlobal(
    Location loc,
    const bool export_,
    ClassType::Ptr type,
    ClassType::Ptr base_type,
    std::vector<ClassMember> members,
    std::vector<ClassFunction> functions)
    : Global(std::move(loc)),
      m_Export(export_),
      m_Opaque(false),
      m_Type(std::move(type)),
      m_BaseType(std::move(base_type)),
      m_Members(std::move(members)),
      m_Functions(std::move(functions))
{
}

void llove::ClassGlobal::Gen(Builder &builder) const try
{
    if (m_Opaque)
        return;

    m_Type->SetBaseType(m_BaseType);

    std::vector<ClassMemberReference> class_members;
    for (auto &member : m_Members)
        class_members.emplace_back(member.Info, member.Name);
    m_Type->SetMembers(std::move(class_members));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (auto &parameter : function.Parameters)
            parameters.emplace_back(parameter.Info);
        class_functions.emplace_back(
            function.Expose,
            function.Virtual,
            function.Override,
            function.Implicit,
            function.Mutable,
            function.Name,
            parameters,
            function.Variadic.first,
            function.Result);
    }
    m_Type->SetFunctions(std::move(class_functions));

    for (auto &function : m_Functions)
        builder.GenFunction(
            {
                .Loc = function.Loc,
                .Register = true,
                .Export = m_Export,
                .Virtual = function.Virtual,
                .Override = function.Override,
                .Interface = false,
                .Implicit = function.Implicit,
                .Class = m_Type,
                .Mutable = function.Mutable,
                .Expose = function.Expose,
                .Name = function.Name,
                .Parameters = function.Parameters,
                .Variadic = function.Variadic,
                .Result = function.Result,
                .Content = nullptr,
            }
        );

    for (auto &function : m_Functions)
        builder.GenFunction(
            {
                .Loc = function.Loc,
                .Register = true,
                .Export = m_Export,
                .Virtual = function.Virtual,
                .Override = function.Override,
                .Interface = false,
                .Implicit = function.Implicit,
                .Class = m_Type,
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
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::pair<std::string, llove::ValuePtr> llove::ClassGlobal::GenImport(
    Context &context,
    Builder &builder,
    const std::string &as,
    const std::map<std::string, std::string> &symbols) const
{
    if (!m_Export)
        return {};

    auto &name = m_Type->GetName();

    if (!(as.empty() && symbols.empty() || symbols.contains(name)))
        return {};

    context.GetParent()->Set(m_Type->Mangle(), m_Type);
    context.GetParent()->SetNamed(symbols.contains(name) ? symbols.at(name) : name, m_Type);

    if (m_Opaque)
        return {};

    std::vector<ClassMemberReference> class_members;
    for (auto &member : m_Members)
        class_members.emplace_back(member.Info, member.Name);
    m_Type->SetMembers(std::move(class_members));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (auto &parameter : function.Parameters)
            parameters.emplace_back(parameter.Info);
        class_functions.emplace_back(
            function.Expose,
            function.Virtual,
            function.Override,
            function.Implicit,
            function.Mutable,
            function.Name,
            parameters,
            function.Variadic.first,
            function.Result);
    }
    m_Type->SetFunctions(std::move(class_functions));

    for (auto &function : m_Functions)
        builder.GenFunction(
            {
                .Loc = function.Loc,
                .Register = true,
                .Export = m_Export,
                .Virtual = function.Virtual,
                .Override = function.Override,
                .Interface = false,
                .Implicit = function.Implicit,
                .Class = m_Type,
                .Mutable = function.Mutable,
                .Expose = function.Expose,
                .Name = function.Name,
                .Parameters = function.Parameters,
                .Variadic = function.Variadic,
                .Result = function.Result,
                .Content = nullptr,
            }
        );

    return {};
}

std::ostream &llove::ClassGlobal::Print(std::ostream &stream) const
{
    stream << "class " << m_Type->GetName();
    if (m_Opaque)
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
