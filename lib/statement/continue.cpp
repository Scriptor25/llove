#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::ContinueStatement::ContinueStatement(Location loc)
    : Statement(std::move(loc))
{
}

void llove::ContinueStatement::Gen(Builder &builder) const try
{
    const auto head_block = builder.GetHead();
    Assert(head_block != nullptr, "no head block in frame");
    builder.CreateBranch(head_block);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::ContinueStatement::Reflect(Context & /* context */) const try
{
    return std::make_unique<ContinueStatement>(m_Loc);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::ContinueStatement::Print(std::ostream &stream) const
{
    return stream << "continue;";
}
