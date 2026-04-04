#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::SwitchExpression::SwitchExpression(
    Location loc,
    ExpressionPtr condition,
    std::vector<SwitchExpressionCase> cases)
    : Expression(std::move(loc)),
      m_Condition(std::move(condition)),
      m_Cases(std::move(cases))
{
}

llove::ValuePtr llove::SwitchExpression::GenVal(
    Builder &builder,
    TypePtr expect) const try
{
    const auto parent = builder.GetParent();
    const auto default_block = builder.CreateBlock("default", parent);
    const auto tail_block = builder.CreateBlock("tail");

    const auto condition = m_Condition->GenVal(builder, nullptr);
    const auto block = builder.GetInsertBlock();

    const auto condition_type = condition->GetType();

    builder.EmitLoc(m_Loc);
    builder.PushFrame(m_Loc, nullptr, tail_block);

    std::map<llvm::ConstantInt *, llvm::BasicBlock *> cases;
    std::map<llvm::BasicBlock *, llvm::Value *> nodes;

    TypePtr type;
    for (auto &[is_default, keys, content] : m_Cases)
    {
        if (!is_default)
            continue;

        builder.SetInsertPoint(default_block);
        const auto value = content->GenVal(builder, std::move(expect));
        nodes.emplace(builder.GetInsertBlock(), value->Load(builder));
        builder.CreateBranch(tail_block);

        type = value->GetType();
        break;
    }

    for (auto &[is_default, keys, content] : m_Cases)
    {
        auto case_block = is_default ? default_block : builder.CreateBlock("case", parent);

        for (auto &key : keys)
        {
            auto key_value = key->GenVal(builder, condition_type);
            key_value = builder.CreateCast(key_value, condition_type, true);

            auto key_const = llvm::dyn_cast<llvm::ConstantInt>(key_value->Load(builder));
            Assert(key_const != nullptr, "invalid non-constant or non-integer case key value");

            cases.emplace(key_const, case_block);
        }

        if (!is_default)
        {
            builder.SetInsertPoint(case_block);
            auto value = content->GenVal(builder, type);
            value = builder.CreateCast(std::move(value), type, true);
            nodes.emplace(builder.GetInsertBlock(), value->Load(builder));
            builder.CreateBranch(tail_block);
        }
    }

    builder.SetInsertPoint(block);
    builder.CreateSwitch(condition->Load(builder), default_block, cases);

    builder.PopFrame();

    tail_block->insertInto(parent);
    builder.SetInsertPoint(tail_block);

    const auto value = builder.CreatePHI(type->GenIR(builder), nodes);
    return Value::CreateR(type, value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::SwitchExpression::Reflect(Context &context) const try
{
    ExpressionPtr condition;
    std::vector<SwitchExpressionCase> cases;

    m_Condition->Reflect(context, condition);

    for (auto &[is_default_, keys_, content_] : m_Cases)
    {
        auto &[is_default, keys, content] = cases.emplace_back();
        is_default = is_default_;
        for (auto &key_ : keys_)
            if (auto &key = keys.emplace_back(); key_)
                key_->Reflect(context, key);
        content_->Reflect(context, content);
    }

    return std::make_unique<SwitchExpression>(m_Loc, std::move(condition), std::move(cases));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::SwitchExpression::Print(std::ostream &stream) const
{
    const auto cur = std::string(PrintDepth += 2, ' ');

    stream << "switch (" << m_Condition << ") {" << std::endl;
    for (auto &[is_default, keys, content] : m_Cases)
    {
        stream << cur << '[';
        if (is_default)
            stream << "default";
        for (auto i = keys.begin(); i != keys.end(); ++i)
        {
            if (is_default || i != keys.begin())
                stream << ", ";
            stream << *i;
        }
        stream << "] -> " << content << ';' << std::endl;
    }
    return stream << std::string(PrintDepth -= 2, ' ') << '}';
}
