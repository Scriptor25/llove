#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::NullExpression::NullExpression(Location loc, PointerType::Ptr type)
    : Expression(std::move(loc)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::NullExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    auto type = m_Type
                    ? m_Type
                    : expect && expect->IsPointer()
                    ? As<PointerType>(std::move(expect))
                    : builder.GetContext().GetPointer(false);

    builder.EmitLoc(m_Loc);

    const auto value = llvm::ConstantPointerNull::get(type->GenIR(builder));
    return Value::CreateR(std::move(type), value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::NullExpression::Reflect(Context &context) const try
{
    PointerType::Ptr type;
    Type::Reflect(context, m_Type, type);

    return std::make_unique<NullExpression>(m_Loc, std::move(type));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::NullExpression::Print(std::ostream &stream) const
{
    stream << "null";
    if (m_Type)
        stream << ':' << m_Type->GetBase();
    return stream;
}
