#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SizeofExpression::SizeofExpression(Location loc, TypePtr type)
    : Expression(std::move(loc)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::SizeofExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    const auto size = m_Type->SizeBits(builder);
    const auto size_type = builder.GetTypes().GetInteger(false, 64);

    builder.EmitLoc(m_Loc);

    return Value::CreateR(size_type, llvm::ConstantInt::get(size_type->GenIR(builder), size >> 3));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::SizeofExpression::Reflect(Builder &builder) const try
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(builder, type);

    return std::make_unique<SizeofExpression>(m_Loc, std::move(type));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

std::ostream &llove::SizeofExpression::Print(std::ostream &stream) const
{
    return stream << "sizeof " << m_Type;
}
