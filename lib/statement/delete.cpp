#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::DeleteStatement::DeleteStatement(Location loc, ExpressionPtr value)
    : Statement(std::move(loc)),
      m_Value(std::move(value))
{
}

void llove::DeleteStatement::Gen(Builder &builder) const try
{
    builder.EmitLoc(m_Loc);

    auto value = m_Value->GenVal(builder, nullptr);
    const auto type = value->GetType();

    if (!type->IsClass())
        return;

    auto class_type = As<ClassType>(type);
    if (const auto destructor = class_type->GetDestructor())
    {
        if (!value->IsReferenceable())
        {
            const auto pointer = builder.CreateAlloca(type->GenIR(builder));
            builder.CreateStore(value->Load(builder), pointer);
            value = Value::CreateL(type, pointer, false);
        }

        const auto function = builder.GenFunction(
            {
                .Class = std::move(class_type),
                .Mutable = destructor->Mutable,
                .Expose = destructor->Expose,
                .Name = destructor->Name,
                .Result = destructor->Result,
            });

        builder.EmitLoc(m_Loc);
        builder.CreateCall(function, {}, std::move(value));
    }
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::DeleteStatement::Reflect(Context &context) const try
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(context, value);

    return std::make_unique<DeleteStatement>(m_Loc, std::move(value));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::DeleteStatement::Print(std::ostream &stream) const
{
    return stream << "delete " << m_Value << ';';
}
