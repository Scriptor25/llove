#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::ClassGlobal::ClassGlobal(Location loc, const bool export_, ClassType::Ptr type)
    : Global(std::move(loc)),
      m_Export(export_),
      m_Type(std::move(type)),
      m_Opaque(true)
{
}

llove::ClassGlobal::ClassGlobal(
    Location loc,
    const bool export_,
    ClassType::Ptr type,
    std::vector<ClassField> fields,
    std::vector<ClassFunction> functions)
    : Global(std::move(loc)),
      m_Export(export_),
      m_Type(std::move(type)),
      m_Opaque(false),
      m_Fields(std::move(fields)),
      m_Functions(std::move(functions))
{
}

void llove::ClassGlobal::Gen(Builder &builder) const try
{
    if (m_Opaque)
        return;

    std::vector<ClassFieldReference> class_fields;
    for (auto &field : m_Fields)
        class_fields.emplace_back(field.Info, field.Name);
    m_Type->SetMembers(std::move(class_fields));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (auto &parameter : function.Parameters)
            parameters.emplace_back(parameter.Info);
        class_functions.emplace_back(
            function.Expose,
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
                .Export = m_Export,
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
                .Export = m_Export,
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

    std::vector<ClassFieldReference> class_fields;
    for (auto &field : m_Fields)
        class_fields.emplace_back(field.Info, field.Name);
    m_Type->SetMembers(std::move(class_fields));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (auto &parameter : function.Parameters)
            parameters.emplace_back(parameter.Info);
        class_functions.emplace_back(
            function.Expose,
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
                .Export = m_Export,
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

    const auto cur = std::string(PrintDepth += 2, ' ');

    stream << " {" << std::endl;
    for (auto &function : m_Functions)
        stream << cur << function << std::endl;
    if (!m_Functions.empty() && !m_Fields.empty())
        stream << std::endl;
    for (auto &field : m_Fields)
        stream << cur << field << std::endl;
    return stream << std::string(PrintDepth -= 2, ' ') << '}';
}
