#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::ClassGlobal::ClassGlobal(ClassType::Ptr type)
    : m_Type(std::move(type)),
      m_Opaque(true)
{
}

llove::ClassGlobal::ClassGlobal(
    ClassType::Ptr type,
    std::vector<ClassField> fields,
    std::vector<ClassFunction> functions)
    : m_Type(std::move(type)),
      m_Opaque(false),
      m_Fields(std::move(fields)),
      m_Functions(std::move(functions))
{
}

void llove::ClassGlobal::Gen(Builder &builder) const
{
    if (m_Opaque)
        return;

    std::vector<ClassFieldReference> class_fields;
    for (auto &[info, name, value, arguments] : m_Fields)
        class_fields.emplace_back(info, name);
    m_Type->SetFields(builder, std::move(class_fields));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info, name] : function.Parameters)
            parameters.emplace_back(info);
        class_functions.emplace_back(
            function.Expose,
            function.Mutable,
            function.Name,
            parameters,
            function.VarArg,
            function.Result);
    }
    m_Type->SetFunctions(std::move(class_functions));

    for (auto &[
             expose,
             mutable_,
             name,
             parameters,
             vararg,
             result,
             content
         ] : m_Functions)
        builder.GenFunction(
            {
                .Class = m_Type,
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
         ] : m_Functions)
        builder.GenFunction(
            {
                .Class = m_Type,
                .Mutable = mutable_,
                .Expose = expose,
                .Name = name,
                .Parameters = parameters,
                .VarArg = vararg,
                .Result = result,
                .Content = content.get(),
            }
        );
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
