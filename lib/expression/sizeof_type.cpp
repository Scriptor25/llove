#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SizeofTypeExpression::SizeofTypeExpression(Location loc, TypePtr type)
    : Expression(std::move(loc)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::SizeofTypeExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    const auto size = m_Type->SizeBits(builder);
    const auto size_type = builder.GetTypes().GetInteger(false, 64);

    builder.EmitLoc(m_Loc);

    return Value::CreateR(size_type, llvm::ConstantInt::get(size_type->Gen(builder), size >> 3));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::SizeofTypeExpression::Reflect(Builder &builder) const try
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<SizeofTypeExpression>(m_Loc, std::move(type));
}
catch (const ErrorStack *cause)
{
    throw new ErrorStack(cause, m_Loc, std::nullopt);
}

std::ostream &llove::SizeofTypeExpression::Print(std::ostream &stream) const
{
    return stream << "sizeof " << m_Type;
}
