#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::ClassGlobal::ClassGlobal(ClassType::Ptr type)
    : m_Type(std::move(type)),
      m_Opaque(true)
{
}

llove::ClassGlobal::ClassGlobal(
    ClassType::Ptr type,
    std::vector<ClassFieldReference> fields,
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
    for (auto &[info_, name_] : m_Fields)
        class_fields.emplace_back(info_, name_);
    m_Type->SetFields(builder, std::move(class_fields));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info_, name_] : function.Parameters)
            parameters.emplace_back(info_);
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
             expose_,
             mutable_,
             name_,
             parameters_,
             vararg_,
             result_,
             content_
         ] : m_Functions)
        builder.GenFunction(
            {
                .Class = m_Type,
                .Mutable = mutable_,
                .Expose = expose_,
                .Name = name_,
                .Parameters = parameters_,
                .VarArg = vararg_,
                .Result = result_,
            }
        );

    for (auto &[
             expose_,
             mutable_,
             name_,
             parameters_,
             vararg_,
             result_,
             content_
         ] : m_Functions)
        builder.GenFunction(
            {
                .Class = m_Type,
                .Mutable = mutable_,
                .Expose = expose_,
                .Name = name_,
                .Parameters = parameters_,
                .VarArg = vararg_,
                .Result = result_,
                .Content = content_.get(),
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
