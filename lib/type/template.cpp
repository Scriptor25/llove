#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::TemplateType::TemplateType(std::string name)
    : m_Name(std::move(name))
{
    Assert(!m_Name.empty(), "name must not be empty");
}

llove::TypeId llove::TemplateType::GetId() const
{
    return ID;
}

bool llove::TemplateType::IsTemplate() const
{
    return true;
}

unsigned llove::TemplateType::SizeBits(Builder& /* builder */)
{
    Error("template type '{}' does not have a size", m_Name);
}

llvm::Type* llove::TemplateType::GenIR(Builder& /* builder */)
{
    Error("template type '{}' does not have an intermediate representation", m_Name);
}

llvm::DIType* llove::TemplateType::GenDI(Builder& /* builder */)
{
    Error("template type '{}' does not have debug information", m_Name);
}

llove::TypePtr llove::TemplateType::Reflect(Context& context) const
{
    if (context.IsInstantiating())
        return context.GetTemplateArgument(m_Name);
    return nullptr;
}

bool llove::TemplateType::TypeInfo(
    Builder& /* builder */,
    std::vector<llvm::Constant*>& /* dst */) const
{
    Error("template type '{}' does not have typeinfo", m_Name);
}

std::string llove::TemplateType::Mangle() const
{
    return 't' + std::to_string(m_Name.size()) + '_' + m_Name;
}

std::ostream& llove::TemplateType::Print(std::ostream& stream) const
{
    return stream << m_Name;
}

llove::InstanceType::InstanceType(
    std::string name,
    std::vector<TypePtr> arguments)
    : m_Name(std::move(name)),
      m_Arguments(std::move(arguments))
{
}

llove::TypeId llove::InstanceType::GetId() const
{
    return ID;
}

bool llove::InstanceType::IsTemplate() const
{
    return true;
}

unsigned llove::InstanceType::SizeBits(Builder& /* builder */)
{
    Error("template type '{}' does not have a size", m_Name);
}

llvm::Type* llove::InstanceType::GenIR(Builder& /* builder */)
{
    Error("template type '{}' does not have an intermediate representation", m_Name);
}

llvm::DIType* llove::InstanceType::GenDI(Builder& /* builder */)
{
    Error("template type '{}' does not have debug information", m_Name);
}

bool llove::InstanceType::TypeInfo(
    Builder& /* builder */,
    std::vector<llvm::Constant*>& /* dst */) const
{
    Error("template type '{}' does not have typeinfo", m_Name);
}

std::string llove::InstanceType::Mangle() const
{
    std::string arguments;
    for (auto& argument : m_Arguments)
        arguments += argument->Mangle();

    return 't' + std::to_string(m_Name.size()) + '_' + m_Name
         + std::to_string(m_Arguments.size()) + '_' + arguments;
}

std::ostream& llove::InstanceType::Print(std::ostream& stream) const
{
    stream << '<';
    for (auto it = m_Arguments.begin(); it != m_Arguments.end(); ++it)
    {
        if (it != m_Arguments.begin())
            stream << ", ";
        stream << *it;
    }
    return stream << "> " << m_Name;
}

llove::TypePtr llove::InstanceType::Reflect(Context& context) const
{
    auto& instance = context.InstantiateTemplate<TypeTemplateInstance>(m_Name, m_Arguments);
    return instance.GetType();
}
