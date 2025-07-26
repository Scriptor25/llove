#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::DeleteStatement::DeleteStatement(ExpressionPtr value)
    : m_Value(std::move(value))
{
}

void llove::DeleteStatement::Gen(Builder &builder) const
{
    const auto value = m_Value->GenVal(builder, nullptr);
    const auto type = value->GetType();

    if (!type->IsClass())
        return;

    auto class_type = As<ClassType>(type);
    if (const auto destructor = class_type->GetDestructor())
    {
        const auto &reference = builder.GenFunction(
            {
                .Class = std::move(class_type),
                .Mutable = destructor->Mutable,
                .Expose = destructor->Expose,
                .Name = destructor->Name,
                .Result = destructor->Result,
            });

        llvm::Value *pointer;
        if (value->IsReferenceable())
        {
            pointer = value->GetPointer();
        }
        else
        {
            pointer = builder.CreateAlloca(type);
            builder.CreateStore(pointer, value);
        }

        builder.CreateCall(reference.Type, reference.Callee, { pointer });
    }
}

llove::StatementPtr llove::DeleteStatement::Reflect(Context &types) const
{
    ExpressionPtr value;

    if (m_Value)
        m_Value->Reflect(types, value);

    return std::make_unique<DeleteStatement>(std::move(value));
}

std::ostream &llove::DeleteStatement::Print(std::ostream &stream) const
{
    return stream << "delete " << m_Value << ';';
}
