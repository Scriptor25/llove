#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::NullExpression::NullExpression(TypePtr type)
    : m_Type(std::move(type))
{
}

llove::ValuePtr llove::NullExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    PointerType::Ptr type;
    if (m_Type)
        type = builder.GetTypes().GetPointer(m_Type, false);
    else if (auto pointer_type = As<PointerType>(expect))
        type = std::move(pointer_type);
    else
        type = builder.GetTypes().GetPointer(false);
    const auto value = llvm::ConstantPointerNull::get(type->Gen(builder));
    return Value::CreateR(std::move(type), value);
}

llove::StatementPtr llove::NullExpression::Reflect(Context &types) const
{
    TypePtr type;
    if (m_Type)
        m_Type->Reflect(types, type);

    return std::make_unique<NullExpression>(std::move(type));
}

std::ostream &llove::NullExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << "null:" << m_Type;
    return stream << "null";
}
