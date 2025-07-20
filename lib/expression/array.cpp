#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ArrayExpression::ArrayExpression(std::vector<ExpressionPtr> values, ArrayType::Ptr type)
    : m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::ArrayExpression::GenVal(Builder &builder, TypePtr expect) const
{
    auto type = m_Type ? m_Type : As<ArrayType>(std::move(expect));
    Assert(type != nullptr, "untyped array expression");

    const auto base = type->GetBase();
    const Field field
    {
        .Mutable = false,
        .Reference = false,
        .Type = base,
    };

    llvm::Value *aggregate = llvm::Constant::getNullValue(type->Gen(builder));

    for (unsigned index = 0; index < m_Values.size(); ++index)
    {
        auto gen_val = m_Values.at(index)->GenVal(builder, base);
        const auto val = field.GenCast(builder, std::move(gen_val));

        aggregate = builder.CreateInsertValue(aggregate, val, index);
    }

    return Value::CreateR(std::move(type), aggregate);
}

std::ostream &llove::ArrayExpression::Print(std::ostream &stream) const
{
    stream << "[ ";
    for (auto i = m_Values.begin(); i != m_Values.end(); ++i)
    {
        if (i != m_Values.begin())
            stream << ", ";
        stream << *i;
    }
    stream << " ]";
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}
