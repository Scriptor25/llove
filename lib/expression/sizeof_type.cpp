#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SizeofTypeExpression::SizeofTypeExpression(Location loc, TypePtr type)
    : Expression(std::move(loc)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::SizeofTypeExpression::GenVal(Builder &builder, TypePtr expect) const
{
    const auto size = m_Type->SizeBits(builder);
    const auto size_type = builder.GetTypes().GetInteger(false, 64);

    builder.EmitLoc(m_Loc);

    return Value::CreateR(size_type, llvm::ConstantInt::get(size_type->Gen(builder), size >> 3));
}

llove::StatementPtr llove::SizeofTypeExpression::Reflect(Builder &builder) const
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<SizeofTypeExpression>(m_Loc, std::move(type));
}

std::ostream &llove::SizeofTypeExpression::Print(std::ostream &stream) const
{
    return stream << "sizeof " << m_Type;
}
