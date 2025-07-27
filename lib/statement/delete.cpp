#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::DeleteStatement::DeleteStatement(Location loc, ExpressionPtr value)
    : Statement(std::move(loc)),
      m_Value(std::move(value))
{
}

void llove::DeleteStatement::Gen(Builder &builder) const
{
    builder.EmitLoc(m_Loc);

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

        builder.EmitLoc(m_Loc);
        builder.CreateCall(reference.Type, reference.Callee, { pointer });
    }
}

llove::StatementPtr llove::DeleteStatement::Reflect(Builder &builder) const
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(builder, value);

    return std::make_unique<DeleteStatement>(m_Loc, std::move(value));
}

std::ostream &llove::DeleteStatement::Print(std::ostream &stream) const
{
    return stream << "delete " << m_Value << ';';
}
