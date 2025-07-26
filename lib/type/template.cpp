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

unsigned llove::TemplateType::Size(Builder &builder) const
{
    Error("template");
}

llvm::Type *llove::TemplateType::Gen(Builder &builder) const
{
    Error("template");
}

llove::TypePtr llove::TemplateType::Reflect(Context &types) const
{
    return types.TemplateArgument(m_Name);
}

std::string llove::TemplateType::Mangle() const
{
    return 't' + std::to_string(m_Name.size()) + '_' + m_Name;
}

std::ostream &llove::TemplateType::Print(std::ostream &stream) const
{
    return stream << m_Name;
}
