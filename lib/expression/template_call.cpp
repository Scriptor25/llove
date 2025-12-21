#include "llove/template.hpp"

#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/function.hpp>
#include <llove/tree.hpp>

llove::TemplateCallExpression::TemplateCallExpression(
    Location loc,
    std::vector<TypePtr> type_arguments,
    std::string callee,
    std::vector<ExpressionPtr> arguments)
    : Expression(std::move(loc)),
      m_TypeArguments(std::move(type_arguments)),
      m_Callee(std::move(callee)),
      m_Arguments(std::move(arguments))
{
}

llove::ValuePtr llove::TemplateCallExpression::GenVal(
    Builder& builder,
    TypePtr /* expect */) const
try
{
    auto& instance = builder.GetContext().InstantiateTemplate<FunctionTemplateInstance>(builder, m_Callee, m_TypeArguments);
    auto& callee = instance.GetCallee();

    std::vector<ValuePtr> arguments;
    for (unsigned i = 0; i < m_Arguments.size(); ++i)
        arguments.emplace_back(
            m_Arguments.at(i)->GenVal(builder, callee.Type->GetParameter(i).GetType()));

    return builder.CreateCall(callee, std::move(arguments), nullptr);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::TemplateCallExpression::Reflect(Context& context) const
try
{
    std::vector<TypePtr> type_arguments;
    std::vector<ExpressionPtr> arguments;

    for (auto& type : m_TypeArguments)
        Type::Reflect(context, type, type_arguments.emplace_back());

    for (auto& argument : m_Arguments)
        argument->Reflect(context, arguments.emplace_back());

    return std::make_unique<TemplateCallExpression>(m_Loc, std::move(type_arguments), m_Callee, std::move(arguments));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream& llove::TemplateCallExpression::Print(std::ostream& stream) const
{
    stream << '<';
    for (auto i = m_TypeArguments.begin(); i != m_TypeArguments.end(); ++i)
    {
        if (i != m_TypeArguments.begin())
            stream << ", ";
        stream << *i;
    }
    stream << "> " << m_Callee << '(';
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << ')';
}
