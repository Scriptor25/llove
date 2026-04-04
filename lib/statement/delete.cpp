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

    const auto class_type = As<ClassType>(type);
    if (const auto destructor = class_type->GetDestructor(class_type))
    {
        auto &[parent, function] = *destructor;

        if (!value->IsReference())
        {
            const auto pointer = builder.CreateAlloca(type->GenIR(builder));
            builder.CreateStore(value->Load(builder), pointer);
            value = Value::CreateL(type, pointer, false);
        }

        Function agg;
        agg.IsExport = function.IsExport;
        agg.IsPublic = function.IsPublic;
        agg.IsVirtual = function.IsVirtual;
        agg.IsOverride = function.IsOverride;
        agg.IsMutable = function.IsMutable;
        agg.Class = parent;
        agg.Name = function.Name;
        agg.Result = function.Result;

        const auto reference = builder.GenFunction(agg, false);

        builder.EmitLoc(m_Loc);
        builder.CreateCall(reference, {}, std::move(value));
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
