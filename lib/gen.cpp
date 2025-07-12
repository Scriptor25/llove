#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

void llove::DefinitionGlobal::Gen(Builder &builder) const
{
    std::string name;
    if (m_Interface)
    {
        name = m_Name;
    }
    else
    {
        name = '_'
               + std::to_string(m_Name.size())
               + '_'
               + m_Name
               + (m_VarArg ? "v" : "")
               + std::to_string(m_Parameters.size())
               + '_';
        for (auto &[info_, name_] : m_Parameters)
            name += info_.Mangle();
        name += m_Result.Mangle();
    }

    std::vector<Field> parameters;
    for (auto &[info_, name_] : m_Parameters)
        parameters.emplace_back(info_);

    auto type = builder.GetTypes().GetFunction(parameters, m_VarArg, m_Result);

    builder.CreateFunction(name, type, m_Interface);
}

void llove::ClassDefinitionGlobal::Gen(Builder &builder) const
{
    std::string name = '_'
                       + std::to_string(m_Name.size())
                       + '_'
                       + m_Name
                       + (m_Mutable ? 'm' : 'c')
                       + std::to_string(m_ClassName.size())
                       + '_'
                       + m_ClassName
                       + (m_VarArg ? "v" : "")
                       + std::to_string(m_Parameters.size())
                       + '_';
    for (auto &[info_, name_] : m_Parameters)
        name += info_.Mangle();
    name += m_Result.Mangle();

    std::vector<Field> parameters;
    for (auto &[info_, name_] : m_Parameters)
        parameters.emplace_back(info_);

    const Field self
    {
        .Mutable = true,
        .Type = builder.GetTypes().GetClass(m_ClassName),
    };

    const auto type = builder.GetTypes().GetFunction(
        parameters,
        m_VarArg,
        m_Result,
        self);

    builder.CreateFunction(name, type, false);
}

void llove::ClassGlobal::Gen(Builder &builder) const
{
    if (m_Opaque)
        return;

    const auto type = builder.GetTypes().GetClass(m_Name);

    std::vector<Parameter> fields;
    for (auto &[info_, name_] : m_Fields)
        fields.emplace_back(info_, name_);
    type->Set(std::move(fields));

    m_Functions;
}
