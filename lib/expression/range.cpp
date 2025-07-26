#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::RangeExpression::RangeExpression(ExpressionPtr beg, ExpressionPtr end)
    : m_Beg(std::move(beg)),
      m_End(std::move(end))
{
}

llove::ValuePtr llove::RangeExpression::GenVal(Builder &builder, TypePtr expect) const
{
    TypePtr type;
    if (expect && expect->IsRange())
        type = As<RangeType>(std::move(expect))->GetEntry();

    auto beg = m_Beg->GenVal(builder, std::move(type));
    auto end = m_End->GenVal(builder, beg->GetType());

    auto entry = builder.GetTypes().GetMax(beg->GetType(), end->GetType());
    beg = builder.CreateCast(std::move(beg), entry);
    end = builder.CreateCast(std::move(end), entry);

    auto range_type = builder.GetTypes().GetRange(std::move(entry));

    llvm::Value *aggregate = llvm::Constant::getNullValue(range_type->Gen(builder));
    aggregate = builder.CreateInsertValue(aggregate, beg->Load(builder), 0);
    aggregate = builder.CreateInsertValue(aggregate, end->Load(builder), 1);

    return Value::CreateR(std::move(range_type), aggregate);
}

llove::StatementPtr llove::RangeExpression::Reflect(Context &types) const
{
    ExpressionPtr beg, end;

    if (m_Beg)
        m_Beg->Reflect(types, beg);
    if (m_End)
        m_End->Reflect(types, end);

    return std::make_unique<RangeExpression>(std::move(beg), std::move(end));
}

std::ostream &llove::RangeExpression::Print(std::ostream &stream) const
{
    return stream << m_Beg << ".." << m_End;
}
