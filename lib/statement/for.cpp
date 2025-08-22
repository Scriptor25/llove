#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>

llove::ForStatement::ForStatement(
    Location loc,
    StatementPtr prefix,
    StatementPtr suffix,
    ExpressionPtr condition,
    StatementPtr content)
    : Statement(std::move(loc)),
      m_Prefix(std::move(prefix)),
      m_Suffix(std::move(suffix)),
      m_Condition(std::move(condition)),
      m_Content(std::move(content))
{
}

void llove::ForStatement::Gen(Builder &builder) const try
{
    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    auto loop_block = builder.CreateBlock("loop", parent);
    const auto tail_block = builder.CreateBlock("tail");

    auto use_tail = false;

    builder.EmitLoc(m_Loc);
    builder.PushFrame(m_Loc, head_block, tail_block);

    if (m_Prefix)
        m_Prefix->Gen(builder);

    builder.EmitLoc(m_Loc);
    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);
    if (m_Condition)
    {
        auto condition = m_Condition->GenVal(builder, builder.GetContext().GetInteger(false, 1));
        condition = builder.CreateCast(std::move(condition), builder.GetContext().GetInteger(false, 1), false);

        builder.EmitLoc(m_Loc);
        builder.CreateBranch(condition->Load(builder), loop_block, tail_block);

        use_tail = true;
    }
    else
    {
        builder.EmitLoc(m_Loc);
        builder.CreateBranch(loop_block);
    }

    builder.SetInsertPoint(loop_block);
    m_Content->Gen(builder);
    loop_block = builder.GetInsertBlock();
    if (!loop_block->getTerminator())
    {
        if (m_Suffix)
            m_Suffix->Gen(builder);

        builder.EmitLoc(m_Loc);
        builder.CreateBranch(head_block);
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

    builder.PopFrame();
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::ForStatement::Reflect(Context &context) const try
{
    StatementPtr prefix, suffix, content;
    ExpressionPtr condition;

    if (m_Prefix)
        m_Prefix->Reflect(context, prefix);
    if (m_Suffix)
        m_Suffix->Reflect(context, suffix);
    if (m_Condition)
        m_Condition->Reflect(context, condition);
    if (m_Content)
        m_Content->Reflect(context, content);

    return std::make_unique<ForStatement>(
        m_Loc,
        std::move(prefix),
        std::move(suffix),
        std::move(condition),
        std::move(content));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
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
