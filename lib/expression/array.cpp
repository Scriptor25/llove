#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ArrayExpression::ArrayExpression(std::vector<ExpressionPtr> values, TypePtr type)
    : m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::ArrayExpression::GenVal(Builder &builder, TypePtr expect) const
{
    auto type = m_Type ? As<ArrayType>(m_Type) : expect ? As<ArrayType>(expect) : nullptr;
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

llove::StatementPtr llove::ArrayExpression::Reflect(Context &types) const
{
    std::vector<ExpressionPtr> values(m_Values.size());
    for (unsigned i = 0; i < m_Values.size(); ++i)
        m_Values.at(i)->Reflect(types, values.at(i));

    TypePtr type;

    if (m_Type)
        m_Type->Reflect(types, type);

    return std::make_unique<ArrayExpression>(std::move(values), std::move(type));
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
