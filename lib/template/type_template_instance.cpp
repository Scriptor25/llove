#include <llove/template.hpp>

llove::TypeTemplateInstance::TypeTemplateInstance(TypePtr type)
    : m_Type(std::move(type))
{
}

llove::TypePtr llove::TypeTemplateInstance::GetType() const
{
    return m_Type;
}
