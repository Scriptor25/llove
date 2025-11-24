#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::RetStatement::RetStatement(
    Location loc,
    ExpressionPtr value)
    : Statement(std::move(loc)),
      m_Value(std::move(value))
{
}

void llove::RetStatement::Gen(Builder& builder) const
try
{
    if (!m_Value)
    {
        builder.EmitLoc(m_Loc);
        builder.CallDeferred({}, true);
        builder.CreateRetVoid();
        return;
    }

    builder.EmitLoc(m_Loc);

    const auto result = builder.GetResult();
    const auto value = m_Value->GenVal(builder, result.GetType());
    const auto result_value = result.GenCast(builder, value, true);

    std::set<llvm::Value*> mask;
    if (!result.IsReference() && value->IsReference())
        mask.emplace(value->GetPointer());

    builder.EmitLoc(m_Loc);
    builder.CallDeferred(mask, true);
    builder.CreateRet(result_value);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::RetStatement::Reflect(Context& context) const
try
{
    ExpressionPtr value;

    if (m_Value)
        m_Value->Reflect(context, value);

    return std::make_unique<RetStatement>(m_Loc, std::move(value));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream& llove::RetStatement::Print(std::ostream& stream) const
{
    if (m_Value)
        return stream << "ret " << m_Value << ';';
    return stream << "ret;";
}
