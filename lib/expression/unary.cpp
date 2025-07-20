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

        if (operand->GetType()->GetId() == TypeId_Class)
        {
            const auto class_type = As<ClassType>(operand->GetType());
            const auto functions = class_type->GetConstructors();
            const auto reference = builder.FindFunction(
                functions,
                std::vector{ operand->AsField() },
                class_type,
                Field{ true, true, class_type });

            if (reference.has_value())
            {
                const auto pointer = builder.CreateAlloca(class_type);
                const auto self = Value::CreateL(class_type, pointer, true);

                builder.CreateCall(reference->Type, reference->Callee, std::vector{ std::move(operand) }, self);

                operand = self;
            }
        }

        const auto value = operand->Load(builder);
        return Value::CreateR(operand->GetType(), value);
    }

    if (const auto operator_ = builder.FindOperator(m_Operator, operand->AsField(), m_Suffix))
        return (*operator_)(builder, std::move(operand));

    Error(
        "undefined unary operator {}{}{}",
        m_Suffix ? std::string{} : m_Operator,
        operand->GetType(),
        m_Suffix ? m_Operator : std::string{});
}

std::ostream &llove::UnaryExpression::Print(std::ostream &stream) const
{
    if (m_Suffix)
        return stream << m_Operand << m_Operator;
    return stream << m_Operator << m_Operand;
}
