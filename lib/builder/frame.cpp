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

    CallDestructors({}, false);

    m_Stack.pop_back();
    m_DebugBuilder->PopFrame();
}

void llove::Builder::SetValue(const std::string &name, ValuePtr value)
{
    Assert(!m_Stack.empty(), "stack is empty");

    m_Stack.back().Values[name] = std::move(value);
}

bool llove::Builder::HasValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    for (auto &frame : std::ranges::reverse_view(m_Stack))
        if (frame.Values.contains(name))
            return true;
    return false;
}

llove::ValuePtr llove::Builder::GetValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    for (auto &frame : std::ranges::reverse_view(m_Stack))
        if (frame.Values.contains(name))
            return frame.Values.at(name);
    return nullptr;
}

void llove::Builder::PushDestructor(llvm::Value *self, const FunctionReference &reference)
{
    Assert(!m_Stack.empty(), "stack is empty");

    m_Stack.back().Destructors[self] = reference;
}

void llove::Builder::CallDestructors(const std::set<llvm::Value *> &mask, const bool propagate)
{
    if (const auto block = m_Builder.GetInsertBlock(); !block || block->getTerminator())
        return;

    if (propagate)
    {
        for (auto &frame : std::ranges::reverse_view(m_Stack))
            for (auto &[self, callee] : frame.Destructors)
                if (!mask.contains(self))
                {
                    auto self_value = Value::CreateL(callee.Type->GetSelf()->Type, self, true);
                    CreateCall(callee, {}, std::move(self_value));
                }
    }
    else
    {
        for (auto &frame = m_Stack.back(); auto &[self, callee] : frame.Destructors)
            if (!mask.contains(self))
            {
                auto self_value = Value::CreateL(callee.Type->GetSelf()->Type, self, true);
                CreateCall(callee, {}, std::move(self_value));
            }
    }
}
