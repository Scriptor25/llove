#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::IfStatement::IfStatement(ExpressionPtr condition, StatementPtr then, StatementPtr else_)
    : m_Condition(std::move(condition)),
      m_Then(std::move(then)),
      m_Else(std::move(else_))
{
}

void llove::IfStatement::Gen(Builder &builder) const
{
    const auto parent = builder.GetParent();
    const auto then_block = builder.CreateBlock("then", parent);
    const auto else_block = builder.CreateBlock("else", parent);
    const auto end_block = builder.CreateBlock("end");

    auto use_end = false;

    const auto condition = m_Condition->GenVal(builder, builder.GetTypes().GetInteger(false, 1));
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

llove::StatementPtr llove::IfStatement::Reflect(Context &types) const
{
    ExpressionPtr condition;
    StatementPtr then, else_;

    if (m_Condition)
        m_Condition->Reflect(types, condition);
    if (m_Then)
        m_Then->Reflect(types, then);
    if (m_Else)
        m_Else->Reflect(types, else_);

    return std::make_unique<IfStatement>(std::move(condition), std::move(then), std::move(else_));
}

std::ostream &llove::IfStatement::Print(std::ostream &stream) const
{
    stream << "if (" << m_Condition << ") " << m_Then;
    if (m_Else)
        stream << " else " << m_Else;
    return stream;
}
