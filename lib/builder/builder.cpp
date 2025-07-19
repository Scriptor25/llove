#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

llove::Builder::Builder(Context &types)
    : m_Types(types),
      m_Builder(m_Context),
      m_Module("main", m_Context),
      m_Parent(nullptr)
{
}

llove::Context &llove::Builder::GetTypes() const
{
    return m_Types;
}

std::string llove::Builder::Mangle(
    const bool interface,
    const ClassType::Ptr &class_type,
    const bool mutable_,
    const std::string &name,
    const std::vector<Parameter> &parameters,
    const bool vararg,
    const Field &result)
{
    if (interface)
        return name;

    auto mangled = '?' + std::to_string(name.size()) + '_' + name;

    if (class_type)
    {
        auto &class_name = class_type->GetName();
        mangled += (mutable_ ? 'm' : 'c') + std::to_string(class_name.size()) + '_' + class_name;
    }

    if (vararg)
        mangled += 'v';

    mangled += std::to_string(parameters.size()) + '_';
    for (auto &[info_, name_] : parameters)
        mangled += info_.Mangle();

    return mangled + result.Mangle();
}

void llove::Builder::SetInsertPoint(llvm::BasicBlock *block)
{
    m_Builder.SetInsertPoint(block);
}

void llove::Builder::ClearInsertPoint()
{
    m_Builder.ClearInsertionPoint();
}

bool llove::Builder::NoTerminator() const
{
    return m_Builder.GetInsertBlock()->getTerminator() == nullptr;
}

llvm::Function *llove::Builder::GetParent() const
{
    return m_Parent;
}

const llove::Field &llove::Builder::GetResult() const
{
    return m_Result;
}

llvm::Function *llove::Builder::GetOrCreateFunction(
    const std::string &name,
    const FunctionType::Ptr &type,
    const bool external)
{
    if (const auto function = m_Module.getFunction(name))
        return function;
    return llvm::Function::Create(
        type->Gen(*this),
        external ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
        name,
        m_Module);
}

llvm::BasicBlock *llove::Builder::CreateBlock(const std::string &name, llvm::Function *parent)
{
    return llvm::BasicBlock::Create(m_Context, name, parent);
}

llove::FunctionReference &llove::Builder::PushFunction(
    const bool expose,
    std::string name,
    FunctionType::Ptr type,
    llvm::Function *callee)
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Type != type)
            continue;
        Assert(expose == function.Expose && callee == function.Callee, "function prototype generation mismatch");
        return function;
    }

    return m_Functions.emplace_back(
        FunctionReference
        {
            .Expose = expose,
            .Name = std::move(name),
            .Type = std::move(type),
            .Callee = callee,
        }
    );
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(const std::string &name)
{
    std::vector<FunctionReference> functions;
    for (auto &function : m_Functions)
        if (function.Name == name)
            functions.emplace_back(function);
    return functions;
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(const std::string &name, const Field &self)
{
    std::vector<FunctionReference> functions;
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (!function.Type->HasSelf())
            continue;
        auto &function_self = function.Type->GetSelf();
        if (function_self.Type != self.Type)
            continue;
        if (function_self.Mutable && !self.Mutable)
            continue;
        functions.emplace_back(function);
    }
    return functions;
}

void llove::Builder::PushFrame()
{
    if (m_Stack.empty())
    {
        m_Stack.emplace_back();
        return;
    }

    auto &[
        destructors,
        values
    ] = m_Stack.emplace_back(m_Stack.back());

    destructors.clear();
    for (const auto &key : values | std::views::keys)
        values[key].first = false;
}

void llove::Builder::PopFrame()
{
    Assert(!m_Stack.empty(), "stack is empty");

    if (const auto block = m_Builder.GetInsertBlock())
    {
        if (const auto instruction = block->getTerminator())
            m_Builder.SetInsertPoint(instruction);

        for (auto &[self, callee] : m_Stack.back().Destructors)
            CreateCall(callee, { self });
    }

    m_Stack.pop_back();
}

void llove::Builder::SetValue(const std::string &name, ValuePtr value)
{
    Assert(!m_Stack.empty(), "stack is empty");
    Assert(
        !m_Stack.back().Values.contains(name) || !m_Stack.back().Values.at(name).first,
        "redefining named value in scope");
    m_Stack.back().Values[name] = { true, std::move(value) };
}

llove::ValuePtr llove::Builder::GetValue(const std::string &name) const
{
    Assert(!m_Stack.empty(), "stack is empty");
    if (m_Stack.back().Values.contains(name))
        return m_Stack.back().Values.at(name).second;
    return nullptr;
}

void llove::Builder::PushDestructor(llvm::Value *self, llvm::FunctionCallee callee)
{
    Assert(!m_Stack.empty(), "stack is empty");
    m_Stack.back().Destructors.emplace_back(self, callee);
}

void llove::Builder::PopDestructor(const llvm::Value *self)
{
    Assert(!m_Stack.empty(), "stack is empty");
    for (auto &[destructors, _] : std::ranges::reverse_view(m_Stack))
        for (auto i = destructors.begin(); i != destructors.end(); ++i)
            if (i->Self == self)
            {
                destructors.erase(i);
                return;
            }
}

llvm::Value *llove::Builder::CreateGlobalString(const std::string &value)
{
    return m_Builder.CreateGlobalStringPtr(value, {}, 0, &m_Module);
}

llove::FunctionReference &llove::Builder::GenFunction(const GenericFunction &fn)
{
    const auto mangled = Mangle(
        fn.Interface,
        fn.Class,
        fn.Mutable,
        fn.Name,
        fn.Parameters,
        fn.VarArg,
        fn.Result);

    std::vector<Field> type_parameters;
    for (auto &[info, name] : fn.Parameters)
        type_parameters.emplace_back(info);

    Field self;
    FunctionType::Ptr function_type;

    if (fn.Class)
    {
        self = {
            .Mutable = fn.Mutable,
            .Reference = true,
            .Type = fn.Class,
        };
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result, self);
    }
    else
    {
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result);
    }

    const auto function = GetOrCreateFunction(mangled, function_type, fn.Interface);
    auto &reference = PushFunction(fn.Expose, fn.Name, function_type, function);

    if (!fn.Content)
        return reference;

    m_Parent = function;
    m_Result = fn.Result;

    const auto entry_block = CreateBlock("entry", function);
    m_Builder.SetInsertPoint(entry_block);

    PushFrame();
    GenParameters(function, fn.Parameters, self);
    fn.Content->Gen(*this);
    PopFrame();

    for (auto &block : *function)
    {
        if (block.getTerminator())
            continue;
        if (fn.Result.Type->GetId() == TypeId_Void)
        {
            m_Builder.SetInsertPoint(&block);
            m_Builder.CreateRetVoid();
            continue;
        }
        Error("not all paths yield");
    }

    const auto error = verifyFunction(*function, &llvm::errs());
    Assert(!error, "function has errors");

    return reference;
}

void llove::Builder::GenParameters(
    llvm::Function *function,
    const std::vector<Parameter> &parameters,
    const Field &self)
{
    auto offset = 0u;
    if (self)
    {
        offset = 1u;

        const auto argument = function->getArg(0);
        argument->setName("self");

        SetValue("self", Value::CreateL(self.Type, argument, self.Mutable));
    }

    for (unsigned i = 0; i < function->arg_size() - offset; ++i)
    {
        auto &[info_, name_] = parameters.at(i);

        const auto argument = function->getArg(i + offset);
        argument->setName(name_);

        ValuePtr storage;
        if (info_.Reference)
        {
            storage = Value::CreateL(info_.Type, argument, info_.Mutable);
        }
        else if (info_.Type->GetId() == TypeId_Class)
        {
            const auto pointer = CreateAlloca(info_.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(info_.Type, pointer, info_.Mutable);

            auto class_type = As<ClassType>(info_.Type);
            if (const auto destructor = class_type->GetDestructor())
            {
                const auto &reference = GenFunction(
                    {
                        .Class = std::move(class_type),
                        .Mutable = destructor->Mutable,
                        .Expose = destructor->Expose,
                        .Name = destructor->Name,
                        .Result = destructor->Result,
                    });

                PushDestructor(
                    pointer,
                    {
                        reference.Type->Gen(*this),
                        reference.Callee,
                    });
            }
        }
        else if (info_.Mutable)
        {
            const auto pointer = CreateAlloca(info_.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(info_.Type, pointer, info_.Mutable);
        }
        else
        {
            storage = Value::CreateR(info_.Type, argument);
        }

        SetValue(name_, storage);
    }
}
