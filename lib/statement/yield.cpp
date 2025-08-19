#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::YieldStatement::YieldStatement(Location loc, ExpressionPtr value)
    : Statement(std::move(loc)),
      m_Value(std::move(value))
{
}

void llove::YieldStatement::Gen(Builder &builder) const try
{
    if (!m_Value)
    {
        builder.EmitLoc(m_Loc);
        builder.CallDeferred({}, true);
        builder.CreateRetVoid();
        return;
    }

    builder.EmitLoc(m_Loc);

    auto &result = builder.GetResult();
    const auto value = m_Value->GenVal(builder, result.Type);
    const auto result_value = result.GenCast(builder, value, true);

    std::set<llvm::Value *> mask;
    if (!result.Reference && value->IsReferenceable())
        mask.emplace(value->GetPointer());

    builder.EmitLoc(m_Loc);
    builder.CallDeferred(mask, true);
    builder.CreateRet(result_value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::YieldStatement::Reflect(Context &context) const try
{
    ExpressionPtr value;

    if (m_Value)
        m_Value->Reflect(context, value);

    return std::make_unique<YieldStatement>(m_Loc, std::move(value));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::YieldStatement::Print(std::ostream &stream) const
{
    if (m_Value)
        return stream << "yield " << m_Value << ';';
    return stream << "yield;";
}
