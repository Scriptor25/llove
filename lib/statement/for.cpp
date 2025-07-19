#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::ForStatement::ForStatement(
    StatementPtr prefix,
    StatementPtr suffix,
    ExpressionPtr condition,
    StatementPtr content)
    : m_Prefix(std::move(prefix)),
      m_Suffix(std::move(suffix)),
      m_Condition(std::move(condition)),
      m_Content(std::move(content))
{
}

void llove::ForStatement::Gen(Builder &builder) const
{
    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    const auto loop_block = builder.CreateBlock("loop", parent);
    const auto end_block = builder.CreateBlock("end");

    auto use_end = false;

    builder.PushFrame();

    if (m_Prefix)
        m_Prefix->Gen(builder);
    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);
    if (m_Condition)
    {
        const auto condition = m_Condition->GenVal(builder, builder.GetTypes().GetInteger(false, 1));
        builder.CreateBranch(condition, loop_block, end_block);
        use_end = true;
    }
    else
    {
        builder.CreateBranch(loop_block);
    }

    builder.SetInsertPoint(loop_block);
    m_Content->Gen(builder);
    if (builder.NoTerminator())
    {
        if (m_Suffix)
            m_Suffix->Gen(builder);
        builder.CreateBranch(head_block);
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

    builder.PopFrame();
}

std::ostream &llove::ForStatement::Print(std::ostream &stream) const
{
    stream << "for (";
    if (m_Prefix)
        stream << m_Prefix;
    stream << ';';
    if (m_Condition)
        stream << ' ' << m_Condition;
    stream << ';';
    if (m_Suffix)
        stream << ' ' << m_Suffix;
    return stream << ") " << m_Content;
}
