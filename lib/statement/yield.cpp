#include <llove/builder.hpp>
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
        builder.CreateRetVoid();
        return;
    }

    auto &result = builder.GetResult();
    auto value = m_Value->GenVal(builder, result.Type);

    if (!result.Reference && value->IsReferenceable())
        builder.PopDestructor(value->GetPointer());

    const auto result_value = result.GenCast(builder, std::move(value));

    builder.CreateRet(result_value);
}

std::ostream &llove::YieldStatement::Print(std::ostream &stream) const
{
    if (m_Value)
        return stream << "yield " << m_Value << ';';
    return stream << "yield;";
}
