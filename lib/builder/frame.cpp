#include <ranges>

#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llvm::Function *llove::Builder::GetParent() const
{
    return m_Parent;
}

llove::Field llove::Builder::GetResult() const
{
    return m_Result;
}

llove::ClassType::Ptr llove::Builder::GetClass() const
{
    return m_Class;
}

llvm::BasicBlock *llove::Builder::GetHead() const
{
    Assert(!m_Stack.empty(), "stack is empty");

    return m_Stack.back().Head;
}

llvm::BasicBlock *llove::Builder::GetTail() const
{
    Assert(!m_Stack.empty(), "stack is empty");

    return m_Stack.back().Tail;
}

void llove::Builder::PushFrame(const std::optional<Location> &loc, llvm::BasicBlock *head, llvm::BasicBlock *tail)
{
    if (!m_Stack.empty())
    {
        const auto &frame = m_Stack.back();
        if (!head)
            head = frame.Head;
        if (!tail)
            tail = frame.Tail;
    }

    auto &frame = m_Stack.emplace_back();
    frame.Head = head;
    frame.Tail = tail;

    m_DebugBuilder.PushFrame(loc);
}

void llove::Builder::PushCleanFrame(const std::optional<Location> &loc)
{
    m_Stack.emplace_back();

    m_DebugBuilder.PushFrame(loc);
}

void llove::Builder::PopFrame()
{
    Assert(!m_Stack.empty(), "stack is empty");

    CallDeferred({}, false);

    m_Stack.pop_back();
    m_DebugBuilder.PopFrame();
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
        [&](const Frame &frame)
        {
            return frame.Values.contains(name);
        });
}

llove::ValuePtr llove::Builder::GetValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    for (auto &frame : std::ranges::reverse_view(m_Stack))
        if (auto it = frame.Values.find(name); it != frame.Values.end())
            return it->second;

    Error("undefined value with name '{}'", name);
}

void llove::Builder::DeferAction(llvm::Value *key, std::function<void()> action)
{
    Assert(!m_Stack.empty(), "stack is empty");

    m_Stack.back().Deferred.emplace_back(key, action);
}

void llove::Builder::PushDestructor(llvm::Value *self, FunctionReference reference)
{
    Assert(!m_Stack.empty(), "stack is empty");

    auto action = [this, self, reference]
    {
        auto self_value = Value::CreateL(reference.Type->GetSelf()->GetType(), self, true);
        CreateCall(reference, {}, std::move(self_value));
    };

    DeferAction(self, action);
}

void llove::Builder::PushDestructor(llvm::Value *self, const ClassType::Ptr &class_type)
{
    if (const auto destructor = class_type->GetDestructor(class_type))
    {
        auto &[parent, function] = *destructor;

        Function agg;
        agg.IsExport = function.IsExport;
        agg.IsPublic = function.IsPublic;
        agg.IsVirtual = function.IsVirtual;
        agg.IsOverride = function.IsOverride;
        agg.IsMutable = function.IsMutable;
        agg.Class = parent;
        agg.Name = function.Name;
        agg.Result = function.Result;

        auto reference = GenFunction(agg, false);

        PushDestructor(self, std::move(reference));
    }
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
        for (auto &frame = m_Stack.back(); auto &[key, action] : frame.Deferred)
            if (!mask.contains(key))
                action();
}
