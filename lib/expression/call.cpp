#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::CallExpression::CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> arguments)
    : m_Callee(std::move(callee)),
      m_Arguments(std::move(arguments))
{
}

llove::ValuePtr llove::CallExpression::GenVal(Builder &builder, TypePtr expect) const
{
    auto [functions, self] = m_Callee->GenCallee(builder);

    std::vector<ValuePtr> arguments;
    std::vector<Field> argument_fields;
    for (auto &argument : m_Arguments)
    {
        auto value = argument->GenVal(builder, nullptr);
        arguments.emplace_back(value);
        argument_fields.emplace_back(value->AsField());
    }

    const auto candidate = builder.FindFunction(
        functions,
        argument_fields,
        self ? self->AsField() : Field{});
    Assert(candidate.has_value(), "no suitable candidate");

    return builder.CreateCall(candidate->Type, candidate->Callee, std::move(arguments), std::move(self));
}

llove::StatementPtr llove::CallExpression::Reflect(Context &types) const
{
    ExpressionPtr callee;
    std::vector<ExpressionPtr> arguments(m_Arguments.size());

    if (m_Callee)
        m_Callee->Reflect(types, callee);

    for (unsigned i = 0; i < m_Arguments.size(); i++)
        m_Arguments.at(i)->Reflect(types, arguments.at(i));

    return std::make_unique<CallExpression>(std::move(callee), std::move(arguments));
}

std::ostream &llove::CallExpression::Print(std::ostream &stream) const
{
    stream << m_Callee << '(';
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << ')';
}
