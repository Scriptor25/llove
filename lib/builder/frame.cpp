#include <ranges>
#include <llove/builder.hpp>
#include <llove/error.hpp>

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

void llove::Builder::PushFrame(llvm::DIScope *scope)
{
    if (!scope && !m_Stack.empty())
        scope = m_Stack.back().Scope;
    m_Stack.emplace_back(scope);
}

void llove::Builder::PopFrame()
{
    Assert(!m_Stack.empty(), "stack is empty");

    CallDestructors({}, false);

    m_Stack.pop_back();
}

void llove::Builder::SetValue(const std::string &name, ValuePtr value)
{
    Assert(!m_Stack.empty(), "stack is empty");

    m_Stack.back().Values[name] = std::move(value);
}

bool llove::Builder::HasValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    for (auto &[
             valid,
             destructors,
             values
         ] : std::ranges::reverse_view(m_Stack))
        if (values.contains(name))
            return true;
    return false;
}

llove::ValuePtr llove::Builder::GetValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");

    for (auto &[
             valid,
             destructors,
             values
         ] : std::ranges::reverse_view(m_Stack))
        if (values.contains(name))
            return values.at(name);
    return nullptr;
}

void llove::Builder::PushDestructor(llvm::Value *self, llvm::FunctionCallee callee)
{
    Assert(!m_Stack.empty(), "stack is empty");

    m_Stack.back().Destructors[self] = std::move(callee);
}

void llove::Builder::CallDestructors(const std::set<llvm::Value *> &mask, const bool propagate)
{
    if (const auto block = m_Builder.GetInsertBlock(); !block || block->getTerminator())
        return;

    if (propagate)
    {
        for (auto &[
                 valid,
                 destructors,
                 values
             ] : std::ranges::reverse_view(m_Stack))
            if (valid)
                for (auto &[self, callee] : destructors)
                    if (!mask.contains(self))
                        CreateCall(callee, { self });
    }
    else
    {
        auto &[
            valid,
            destructors,
            values
        ] = m_Stack.back();
        if (valid)
            for (auto &[self, callee] : destructors)
                if (!mask.contains(self))
                    CreateCall(callee, { self });
    }
}
