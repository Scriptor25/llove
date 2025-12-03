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
    return context.TemplateArgument(m_Name);
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

llove::TemplateClassType::TemplateClassType(
    std::string name,
    std::vector<TypePtr> arguments)
    : m_Name(std::move(name)),
      m_Arguments(std::move(arguments))
{
}

bool llove::TemplateClassType::IsInstantiated() const
{
    return m_IsInstantiated;
}

void llove::TemplateClassType::Instantiate()
{
    m_IsInstantiated = true;
}

llove::TypeId llove::TemplateClassType::GetId() const
{
    return ID;
}

bool llove::TemplateClassType::IsTemplate() const
{
    return true;
}

unsigned llove::TemplateClassType::SizeBits(Builder& /* builder */)
{
    Error("template class type '{}' does not have a size", m_Name);
}

llvm::Type* llove::TemplateClassType::GenIR(Builder& /* builder */)
{
    Error("template class type '{}' does not have an intermediate representation", m_Name);
}

llvm::DIType* llove::TemplateClassType::GenDI(Builder& /* builder */)
{
    Error("template class type '{}' does not have debug information", m_Name);
}

llove::TypePtr llove::TemplateClassType::Reflect(Context& context) const
{
    std::vector<TypePtr> arguments;
    for (auto& argument : m_Arguments)
    {
        Type::Reflect(context, argument, arguments.emplace_back());
    }
    return context.InstantiateClass(m_Name, std::move(arguments), false);
}

bool llove::TemplateClassType::TypeInfo(
    Builder& /* builder */,
    std::vector<llvm::Constant*>& /* dst */) const
{
    Error("template class type '{}' does not have typeinfo", m_Name);
}

std::string llove::TemplateClassType::Mangle() const
{
    auto result = 't' + std::to_string(m_Name.size()) + '_' + m_Name
                + std::to_string(m_Arguments.size()) + '_';
    for (auto& argument : m_Arguments)
    {
        result += argument->Mangle();
    }
    return result;
}

std::ostream& llove::TemplateClassType::Print(std::ostream& stream) const
{
    stream << "class<";
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
        {
            stream << ", ";
        }
        stream << *i;
    }
    return stream << "> " << m_Name;
}
