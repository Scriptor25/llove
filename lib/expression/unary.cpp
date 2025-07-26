#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::UnaryExpression::UnaryExpression(std::string operator_, ExpressionPtr operand, const bool suffix)
    : m_Operator(std::move(operator_)),
      m_Operand(std::move(operand)),
      m_Suffix(suffix)
{
}

llove::ValuePtr llove::UnaryExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    auto operand = m_Operand->GenVal(builder, expect);

    if (m_Operator == "$")
    {
        Assert(operand->IsReferenceable(), "cannot remove ownership from rvalue");

        if (operand->GetType()->IsClass())
        {
            const auto class_type = As<ClassType>(operand->GetType());
            const auto functions = class_type->GetConstructors();
            const auto reference = builder.FindFunction(
                functions,
                { { false, false, class_type } },
                class_type,
                { true, true, class_type });

            if (reference.has_value())
            {
                const auto pointer = builder.CreateAlloca(class_type);
                const auto self = Value::CreateL(class_type, pointer, true);

                if (operand->IsReferenceable())
                    operand = Value::CreateR(operand->GetType(), operand->Load(builder));

                builder.CreateCall(
                    reference->Type,
                    reference->Callee,
                    { std::move(operand) },
                    self);

                operand = self;
            }
        }

        if (operand->IsReferenceable())
            operand = Value::CreateR(operand->GetType(), operand->Load(builder));
        return operand;
    }

    if (const auto operator_ = builder.FindOperator(m_Operator, operand->AsField(), m_Suffix))
        return (*operator_)(builder, std::move(operand));

    Error(
        "undefined unary operator {}{}{}",
        m_Suffix ? std::string{} : m_Operator,
        operand->GetType(),
        m_Suffix ? m_Operator : std::string{});
}

llove::StatementPtr llove::UnaryExpression::Reflect(Context &types) const
{
    ExpressionPtr operand;

    if (m_Operand)
        m_Operand->Reflect(types, operand);

    return std::make_unique<UnaryExpression>(m_Operator, std::move(operand), m_Suffix);
}

std::ostream &llove::UnaryExpression::Print(std::ostream &stream) const
{
    if (m_Suffix)
        return stream << m_Operand << m_Operator;
    return stream << m_Operator << m_Operand;
}
