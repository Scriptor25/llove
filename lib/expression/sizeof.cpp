#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SizeofExpression::SizeofExpression(Location loc, TypePtr type)
    : Expression(std::move(loc)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::SizeofExpression::GenVal(Builder &builder, const TypePtr expect) const try
{
    const auto size = m_Type->SizeBits(builder);
    const auto size_type = expect && expect->IsInteger()
                               ? As<IntegerType>(expect)
                               : builder.GetContext().GetInteger(false, 64);

    const auto bytes = size / 8 + (size % 8 != 0);

    builder.EmitLoc(m_Loc);

    return Value::CreateR(size_type, llvm::ConstantInt::get(size_type->GenIR(builder), bytes));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::SizeofExpression::Reflect(Context &context) const try
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    return std::make_unique<SizeofExpression>(m_Loc, std::move(type));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::SizeofExpression::Print(std::ostream &stream) const
{
    return stream << "sizeof " << m_Type;
}
