#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::StructExpression::StructExpression(std::map<std::string, ExpressionPtr> values, StructType::Ptr type)
    : m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::StructExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    auto type = m_Type ? m_Type : As<StructType>(expect);
    Assert(type != nullptr, "untyped struct expression");

    llvm::Value *aggregate = llvm::Constant::getNullValue(type->Gen(builder));

    for (auto &[key, value] : m_Values)
    {
        const auto index = type->GetFieldIndex(key);
        auto &field = type->GetField(index);

        auto gen_val = value->GenVal(builder, field.Type);
        const auto val = field.GenCast(builder, std::move(gen_val));

        aggregate = builder.CreateInsertValue(aggregate, val, index);
    }

    return Value::CreateR(std::move(type), aggregate);
}

std::ostream &llove::StructExpression::Print(std::ostream &stream) const
{
    stream << "{ ";
    for (auto i = m_Values.begin(); i != m_Values.end(); ++i)
    {
        if (i != m_Values.begin())
            stream << ", ";
        stream << i->first << ": " << i->second;
    }
    stream << " }";
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}
