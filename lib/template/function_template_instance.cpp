#include <llove/function.hpp>
#include <llove/template.hpp>

llove::FunctionTemplateInstance::FunctionTemplateInstance(FunctionReference callee)
    : m_Callee(std::move(callee))
{
}

const llove::FunctionReference& llove::FunctionTemplateInstance::GetCallee() const
{
    return m_Callee;
}
