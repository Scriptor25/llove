#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::WhileStatement::WhileStatement(
    Location loc,
    ExpressionPtr condition,
    StatementPtr content)
    : Statement(std::move(loc)),
      m_Condition(std::move(condition)),
      m_Content(std::move(content))
{
}

void llove::WhileStatement::Gen(Builder& builder) const
{
    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    auto loop_block = builder.CreateBlock("loop", parent);
    const auto tail_block = builder.CreateBlock("tail", parent);

    builder.EmitLoc(m_Loc);
    builder.PushFrame(m_Loc, head_block, tail_block);

    builder.EmitLoc(m_Loc);
    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);

    auto condition = m_Condition->GenVal(builder, builder.GetContext().GetBoolean());
    condition = builder.CreateCast(std::move(condition), builder.GetContext().GetBoolean(), false);

    builder.EmitLoc(m_Loc);
    builder.CreateBranch(condition->Load(builder), loop_block, tail_block);

    builder.SetInsertPoint(loop_block);
    m_Content->Gen(builder);
    loop_block = builder.GetInsertBlock();
    if (!loop_block->getTerminator())
    {
        builder.EmitLoc(m_Loc);
        builder.CreateBranch(head_block);
    }

    builder.SetInsertPoint(tail_block);

    builder.PopFrame();
}

llove::StatementPtr llove::WhileStatement::Reflect(Context& context) const
{
    ExpressionPtr condition;
    StatementPtr content;

    if (m_Condition)
        m_Condition->Reflect(context, condition);
    if (m_Content)
        m_Content->Reflect(context, content);

    return std::make_unique<WhileStatement>(m_Loc, std::move(condition), std::move(content));
}

std::ostream& llove::WhileStatement::Print(std::ostream& stream) const
{
    return stream << "while (" << m_Condition << ") " << m_Content;
}
