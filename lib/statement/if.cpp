#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::IfStatement::IfStatement(Location loc, ExpressionPtr condition, StatementPtr then, StatementPtr else_)
    : Statement(std::move(loc)),
      m_Condition(std::move(condition)),
      m_Then(std::move(then)),
      m_Else(std::move(else_))
{
}

void llove::IfStatement::Gen(Builder &builder) const try
{
    const auto parent = builder.GetParent();
    auto then_block = builder.CreateBlock("then", parent);
    auto else_block = builder.CreateBlock("else", parent);
    const auto tail_block = builder.CreateBlock("tail");

    auto use_tail = false;

    builder.EmitLoc(m_Loc);

    auto condition = m_Condition->GenVal(builder, builder.GetContext().GetBoolean());
    condition = builder.CreateCast(std::move(condition), builder.GetContext().GetBoolean(), false);

    builder.EmitLoc(m_Loc);
    builder.CreateBranch(condition->Load(builder), then_block, else_block);

    builder.SetInsertPoint(then_block);
    m_Then->Gen(builder);
    then_block = builder.GetInsertBlock();
    if (!then_block->getTerminator())
    {
        builder.EmitLoc(m_Loc);
        builder.CreateBranch(tail_block);
        use_tail = true;
    }

    builder.SetInsertPoint(else_block);
    if (m_Else)
        m_Else->Gen(builder);
    else_block = builder.GetInsertBlock();
    if (!else_block->getTerminator())
    {
        builder.EmitLoc(m_Loc);
        builder.CreateBranch(tail_block);
        use_tail = true;
    }

    if (use_tail)
    {
        tail_block->insertInto(parent);
        builder.SetInsertPoint(tail_block);
    }
    else
    {
        tail_block->deleteValue();
        builder.ClearInsertionPoint();
    }
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::IfStatement::Reflect(Context &context) const try
{
    ExpressionPtr condition;
    StatementPtr then, else_;

    if (m_Condition)
        m_Condition->Reflect(context, condition);
    if (m_Then)
        m_Then->Reflect(context, then);
    if (m_Else)
        m_Else->Reflect(context, else_);

    return std::make_unique<IfStatement>(m_Loc, std::move(condition), std::move(then), std::move(else_));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::IfStatement::Print(std::ostream &stream) const
{
    stream << "if (" << m_Condition << ") " << m_Then;
    if (m_Else)
        stream << " else " << m_Else;
    return stream;
}
