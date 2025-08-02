#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::ClassGlobal::ClassGlobal(Location loc, ClassType::Ptr type)
    : Global(std::move(loc)),
      m_Type(std::move(type)),
      m_Opaque(true)
{
}

llove::ClassGlobal::ClassGlobal(
    Location loc,
    ClassType::Ptr type,
    std::vector<ClassField> fields,
    std::vector<ClassFunction> functions)
    : Global(std::move(loc)),
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
    m_Type->SetFields(builder, std::move(class_fields));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (auto &parameter : function.Parameters)
            parameters.emplace_back(parameter.Info);
        class_functions.emplace_back(
            function.Expose,
            function.Implicit,
            function.Delete,
            function.Mutable,
            function.Name,
            parameters,
            function.VarArg,
            function.Result);
    }
    m_Type->SetFunctions(std::move(class_functions));

    for (auto &function : m_Functions)
        builder.GenFunction(
            {
                // TODO: loc
                .Implicit = function.Implicit,
                .Delete = function.Delete,
                .Class = m_Type,
                .Mutable = function.Mutable,
                .Expose = function.Expose,
                .Name = function.Name,
                .Parameters = function.Parameters,
                .VarArg = function.VarArg,
                .Result = function.Result,
            }
        );

    for (auto &function : m_Functions)
        builder.GenFunction(
            {
                // TODO: loc
                .Implicit = function.Implicit,
                .Delete = function.Delete,
                .Class = m_Type,
                .Mutable = function.Mutable,
                .Expose = function.Expose,
                .Name = function.Name,
                .Parameters = function.Parameters,
                .VarArg = function.VarArg,
                .Result = function.Result,
                .Content = function.Content.get(),
            }
        );
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
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
