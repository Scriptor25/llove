#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::YieldStatement::YieldStatement(ExpressionPtr value)
    : m_Value(std::move(value))
{
}

void llove::YieldStatement::Gen(Builder &builder) const
{
    if (!m_Value)
    {
        builder.CallDestructors({}, true);
        builder.CreateRetVoid();
        return;
    }

    auto &result = builder.GetResult();
    const auto value = m_Value->GenVal(builder, result.Type);
    const auto result_value = result.GenCast(builder, value);

    std::set<llvm::Value *> mask;
    if (!result.Reference && value->IsReferenceable())
        mask.emplace(value->GetPointer());

    builder.CallDestructors(mask, true);
    builder.CreateRet(result_value);
}

std::ostream &llove::YieldStatement::Print(std::ostream &stream) const
{
    if (m_Value)
        return stream << "yield " << m_Value << ';';
    return stream << "yield;";
}
