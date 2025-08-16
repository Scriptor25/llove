#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::TemplateType::TemplateType(std::string name)
    : m_Name(std::move(name))
{
}

llove::TypeId llove::TemplateType::GetId() const
{
    return TypeId_Template;
}

bool llove::TemplateType::IsTemplate() const
{
    return true;
}

unsigned llove::TemplateType::SizeBits(Builder &builder) const
{
    Error("template");
}

llvm::Type *llove::TemplateType::GenIR(Builder &builder)
{
    Error("template");
}

llvm::DIType *llove::TemplateType::GenDI(Builder &builder)
{
    Error("template");
}

llove::TypePtr llove::TemplateType::Reflect(Context &context) const
{
    return context.TemplateArgument(m_Name);
}

std::string llove::TemplateType::Mangle() const
{
    return 't' + std::to_string(m_Name.size()) + '_' + m_Name;
}

std::ostream &llove::TemplateType::Print(std::ostream &stream) const
{
    return stream << m_Name;
}

llove::ClassTemplateType::ClassTemplateType(std::string name, std::vector<TypePtr> arguments)
    : m_Name(std::move(name)),
      m_Arguments(std::move(arguments))
{
}

llove::TypeId llove::ClassTemplateType::GetId() const
{
    return TypeId_Template;
}

bool llove::ClassTemplateType::IsTemplate() const
{
    return true;
}

unsigned llove::ClassTemplateType::SizeBits(Builder &builder) const
{
    Error("template");
}

llvm::Type *llove::ClassTemplateType::GenIR(Builder &builder)
{
    Error("template");
}

llvm::DIType *llove::ClassTemplateType::GenDI(Builder &builder)
{
    Error("template");
}

llove::TypePtr llove::ClassTemplateType::Reflect(Context &context) const
{
    std::vector<TypePtr> arguments;
    for (auto &argument : m_Arguments)
        Type::Reflect(context, argument, arguments.emplace_back());

    return context.InstantiateTemplateClass(m_Name, arguments);
}

std::string llove::ClassTemplateType::Mangle() const
{
    return 't' + std::to_string(m_Name.size()) + '_' + m_Name + std::to_string(m_Arguments.size()) + '_';
}

std::ostream &llove::ClassTemplateType::Print(std::ostream &stream) const
{
    stream << "class<";
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << "> " << m_Name;
}
