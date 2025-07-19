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
    const auto type = m_Type ? m_Type : As<StructType>(expect);
    Assert(type != nullptr, "untyped struct expression");

    const auto pointer = builder.CreateAlloca(type);
    builder.CreateStore(pointer, llvm::Constant::getNullValue(type->Gen(builder)));

    for (auto &[key_, value_] : m_Values)
    {
        const auto index = type->GetFieldIndex(key_);
        auto &field = type->GetField(index);

        auto value = value_->GenVal(builder, field.Type);
        const auto llvm_value = field.GenCast(builder, std::move(value));

        const auto element_pointer = builder.CreateStructGEP(type, pointer, index);
        builder.CreateStore(element_pointer, llvm_value);
    }

    return Value::CreateL(type, pointer, false);
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
