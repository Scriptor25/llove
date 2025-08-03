#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::RangeExpression::RangeExpression(Location loc, ExpressionPtr beg, ExpressionPtr end)
    : Expression(std::move(loc)),
      m_Beg(std::move(beg)),
      m_End(std::move(end))
{
}

llove::ValuePtr llove::RangeExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    TypePtr type;
    if (expect && expect->IsRange())
        type = As<RangeType>(std::move(expect))->GetEntry();

    auto beg = m_Beg->GenVal(builder, std::move(type));
    auto end = m_End->GenVal(builder, beg->GetType());

    auto entry = builder.GetTypes().TypeUnion(beg->GetType(), end->GetType());
    beg = builder.CreateCast(std::move(beg), entry, true);
    end = builder.CreateCast(std::move(end), entry, true);

    auto range_type = builder.GetTypes().GetRange(std::move(entry));

    builder.EmitLoc(m_Loc);

    llvm::Value *aggregate = llvm::Constant::getNullValue(range_type->GenIR(builder));
    aggregate = builder.CreateInsertValue(aggregate, beg->Load(builder), 0);
    aggregate = builder.CreateInsertValue(aggregate, end->Load(builder), 1);

    return Value::CreateR(std::move(range_type), aggregate);
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::RangeExpression::Reflect(Builder &builder) const try
{
    ExpressionPtr beg;
    if (m_Beg)
        m_Beg->Reflect(builder, beg);

    ExpressionPtr end;
    if (m_End)
        m_End->Reflect(builder, end);

    return std::make_unique<RangeExpression>(m_Loc, std::move(beg), std::move(end));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

std::ostream &llove::RangeExpression::Print(std::ostream &stream) const
{
    return stream << m_Beg << ".." << m_End;
}
