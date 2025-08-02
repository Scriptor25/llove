#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

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
    const auto then_block = builder.CreateBlock("then", parent);
    const auto else_block = builder.CreateBlock("else", parent);
    const auto end_block = builder.CreateBlock("end");

    auto use_end = false;

    builder.EmitLoc(m_Loc);

    auto condition = m_Condition->GenVal(builder, builder.GetTypes().GetInteger(false, 1));
    condition = builder.CreateCast(std::move(condition), builder.GetTypes().GetInteger(false, 1), false);
    builder.CreateBranch(condition, then_block, else_block);

    builder.SetInsertPoint(then_block);
    m_Then->Gen(builder);
    if (builder.NoTerminator())
    {
        builder.CreateBranch(end_block);
        use_end = true;
    }

    builder.SetInsertPoint(else_block);
    if (m_Else)
    {
        m_Else->Gen(builder);
    }
    if (builder.NoTerminator())
    {
        builder.CreateBranch(end_block);
        use_end = true;
    }

    if (use_end)
    {
        end_block->insertInto(parent);
        builder.SetInsertPoint(end_block);
    }
    else
    {
        end_block->deleteValue();
        builder.ClearInsertPoint();
    }
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::IfStatement::Reflect(Builder &builder) const try
{
    ExpressionPtr condition;
    StatementPtr then, else_;

    if (m_Condition)
        m_Condition->Reflect(builder, condition);
    if (m_Then)
        m_Then->Reflect(builder, then);
    if (m_Else)
        m_Else->Reflect(builder, else_);

    return std::make_unique<IfStatement>(m_Loc, std::move(condition), std::move(then), std::move(else_));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

std::ostream &llove::IfStatement::Print(std::ostream &stream) const
{
    stream << "if (" << m_Condition << ") " << m_Then;
    if (m_Else)
        stream << " else " << m_Else;
    return stream;
}
