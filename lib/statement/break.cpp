#include <llove/builder.hpp>
#include <llove/tree.hpp>

llove::BreakStatement::BreakStatement(Location loc)
    : Statement(std::move(loc))
{
}

void llove::BreakStatement::Gen(Builder &builder) const try
{
    const auto tail_block = builder.GetTail();
    Assert(tail_block != nullptr, "no tail block in frame");
    builder.CreateBranch(tail_block);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::BreakStatement::Reflect(Context & /* context */) const try
{
    return std::make_unique<BreakStatement>(m_Loc);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::BreakStatement::Print(std::ostream &stream) const
{
    return stream << "break;";
}
