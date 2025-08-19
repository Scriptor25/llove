#include <ranges>
#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/value.hpp>

llvm::Function *llove::Builder::GetParent() const
{
    return m_Parent;
}

const llove::Field &llove::Builder::GetResult() const
{
    return m_Result;
}

llove::ClassType::Ptr llove::Builder::GetClass() const
{
    return m_Class;
}

void llove::Builder::PushFrame(const std::optional<Location> &loc)
{
    m_Stack.emplace_back();
    m_DebugBuilder->PushFrame(loc);
}

void llove::Builder::PopFrame()
{
    Assert(!m_Stack.empty(), "stack is empty");

    CallDeferred({}, false);

    m_Stack.pop_back();
    m_DebugBuilder->PopFrame();
}

void llove::Builder::SetValue(const std::string &name, ValuePtr value)
{
    Assert(!m_Stack.empty(), "stack is empty");
    Assert(!m_Stack.back().Values.contains(name), "already defined value with name '{}'", name);

    m_Stack.back().Values.emplace(name, std::move(value));
}

bool llove::Builder::HasValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    return std::ranges::any_of(
        m_Stack,
        [&name](auto &frame)
        {
            return frame.Values.contains(name);
        });
}

llove::ValuePtr llove::Builder::GetValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    for (auto &frame : std::ranges::reverse_view(m_Stack))
        if (frame.Values.contains(name))
            return frame.Values.at(name);

    Error("undefined value with name '{}'", name);
}

void llove::Builder::DeferAction(llvm::Value *key, std::function<void()> action)
{
    Assert(!m_Stack.empty(), "stack is empty");

    m_Stack.back().Deferred.emplace_back(key, action);
}

void llove::Builder::PushDestructor(llvm::Value *self, const FunctionReference &callee)
{
    Assert(!m_Stack.empty(), "stack is empty");

    auto action = [this, self, callee]
    {
        auto self_value = Value::CreateL(callee.Type->GetSelf()->Type, self, true);
        CreateCall(callee, {}, std::move(self_value));
    };

    DeferAction(self, action);
}

void llove::Builder::CallDeferred(const std::set<llvm::Value *> &mask, const bool propagate)
{
    Assert(!m_Stack.empty(), "stack is empty");

    if (const auto block = m_LLVMBuilder.GetInsertBlock(); !block || block->getTerminator())
        return;

    if (propagate)
    {
        for (auto &frame : std::ranges::reverse_view(m_Stack))
            for (auto &[key, action] : frame.Deferred)
                if (!mask.contains(key))
                    action();
    }
    else
    {
        for (auto &frame = m_Stack.back(); auto &[key, action] : frame.Deferred)
            if (!mask.contains(key))
                action();
    }
}
