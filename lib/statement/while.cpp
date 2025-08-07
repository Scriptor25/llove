#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::WhileStatement::WhileStatement(Location loc, ExpressionPtr condition, StatementPtr content)
    : Statement(std::move(loc)),
      m_Condition(std::move(condition)),
      m_Content(std::move(content))
{
}

void llove::WhileStatement::Gen(Builder &builder) const
{
    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    const auto loop_block = builder.CreateBlock("loop", parent);
    const auto end_block = builder.CreateBlock("end", parent);

    builder.EmitLoc(m_Loc);
    builder.PushFrame(m_Loc);

    builder.EmitLoc(m_Loc);
    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);

    auto condition = m_Condition->GenVal(builder, builder.GetTypes().GetInteger(false, 1));
    condition = builder.CreateCast(std::move(condition), builder.GetTypes().GetInteger(false, 1), false);

    builder.EmitLoc(m_Loc);
    builder.CreateBranch(condition, loop_block, end_block);

    builder.SetInsertPoint(loop_block);
    m_Content->Gen(builder);

    if (builder.NoTerminator())
    {
        builder.EmitLoc(m_Loc);
        builder.CreateBranch(head_block);
    }

    builder.SetInsertPoint(end_block);

    builder.PopFrame();
}

llove::StatementPtr llove::WhileStatement::Reflect(Builder &builder) const
{
    ExpressionPtr condition;
    StatementPtr content;

    if (m_Condition)
        m_Condition->Reflect(builder, condition);
    if (m_Content)
        m_Content->Reflect(builder, content);

    return std::make_unique<WhileStatement>(m_Loc, std::move(condition), std::move(content));
}

std::ostream &llove::WhileStatement::Print(std::ostream &stream) const
{
    return stream << "while (" << m_Condition << ") " << m_Content;
}
