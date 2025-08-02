#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::NullExpression::NullExpression(Location loc, TypePtr type)
    : Expression(std::move(loc)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::NullExpression::GenVal(Builder &builder, const TypePtr expect) const try
{
    PointerType::Ptr type;
    if (m_Type)
        type = builder.GetTypes().GetPointer(m_Type, false);
    else if (auto pointer_type = As<PointerType>(expect))
        type = std::move(pointer_type);
    else
        type = builder.GetTypes().GetPointer(false);

    builder.EmitLoc(m_Loc);

    const auto value = llvm::ConstantPointerNull::get(type->Gen(builder));
    return Value::CreateR(std::move(type), value);
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::NullExpression::Reflect(Builder &builder) const try
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<NullExpression>(m_Loc, std::move(type));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

std::ostream &llove::NullExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << "null:" << m_Type;
    return stream << "null";
}
