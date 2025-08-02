#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::CallExpression::CallExpression(Location loc, ExpressionPtr callee, std::vector<ExpressionPtr> arguments)
    : Expression(std::move(loc)),
      m_Callee(std::move(callee)),
      m_Arguments(std::move(arguments))
{
}

llove::ValuePtr llove::CallExpression::GenVal(Builder &builder, TypePtr expect) const try
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

    builder.EmitLoc(m_Loc);

    return builder.CreateCall(*candidate, std::move(arguments), std::move(self));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::CallExpression::Reflect(Builder &builder) const try
{
    ExpressionPtr callee;
    if (m_Callee)
        m_Callee->Reflect(builder, callee);

    std::vector<ExpressionPtr> arguments;
    for (auto &argument : m_Arguments)
        argument->Reflect(builder, arguments.emplace_back());

    return std::make_unique<CallExpression>(m_Loc, std::move(callee), std::move(arguments));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
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
