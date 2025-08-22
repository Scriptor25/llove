#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

llove::Builder::Builder(
    Context &context,
    const Machine &machine,
    const bool debug,
    const bool optimized,
    const bool profiling,
    const llvm::DICompileUnit::DebugEmissionKind emission,
    const std::filesystem::path &source_path,
    const std::string &command_line)
    : Builder(
        context,
        machine,
        debug,
        optimized,
        profiling,
        emission,
        source_path,
        command_line,
        source_path.filename().replace_extension().string())
{
}

llove::Builder::Builder(
    Context &context,
    const Machine &machine,
    const bool debug,
    const bool optimized,
    const bool profiling,
    const llvm::DICompileUnit::DebugEmissionKind emission,
    const std::filesystem::path &source_path,
    const std::string &command_line,
    const std::string &module_id)
    : m_Context(context),
      m_Debug(debug),
      m_LLVMBuilder(m_LLVMContext),
      m_LLVMModule(module_id, m_LLVMContext)
{
    m_LLVMModule.setSourceFileName(source_path.string());

    auto target_triple = machine.Triple.empty() ? llvm::sys::getDefaultTargetTriple() : machine.Triple;
    auto cpu = machine.CPU.empty() ? "generic" : machine.CPU;

    std::string features;
    for (auto i = machine.Features.begin(); i != machine.Features.end(); ++i)
    {
        if (i != machine.Features.begin())
            features += ',';
        features += *i;
    }

    std::string target_error;
    const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);
    Assert(target != nullptr, "failed to get target for triple '{}': {}", target_triple, target_error);

    m_TargetMachine = target->createTargetMachine(
        target_triple,
        cpu,
        features,
        machine.Options,
        machine.Relocation);
    Assert(
        m_TargetMachine != nullptr,
        "failed to create target machine for triple '{}', cpu '{}' and features '{}'",
        target_triple,
        cpu,
        features);

    m_LLVMModule.setDataLayout(m_TargetMachine->createDataLayout());
    m_LLVMModule.setTargetTriple(target_triple);

    m_DebugBuilder = std::make_unique<DebugBuilder>(
        m_Debug,
        m_LLVMModule,
        source_path,
        optimized,
        profiling,
        command_line,
        emission);

    m_Stack.emplace_back();
}

llove::Context &llove::Builder::GetContext() const
{
    return m_Context;
}

bool llove::Builder::IsDebug() const
{
    return m_Debug;
}

llove::DebugBuilder &llove::Builder::GetDebug() const
{
    return *m_DebugBuilder;
}

void llove::Builder::EmitLoc(const Location &loc)
{
    m_DebugBuilder->EmitLoc(*this, loc);
}

llvm::LLVMContext &llove::Builder::GetLLVMContext()
{
    return m_LLVMContext;
}

const llvm::DataLayout &llove::Builder::GetDataLayout() const
{
    return m_LLVMModule.getDataLayout();
}

std::string llove::Builder::Mangle(
    const bool interface,
    const ClassType::Ptr &class_type,
    const bool mutable_,
    const std::string &name,
    const std::vector<Parameter> &parameters,
    const bool variadic,
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

    if (variadic)
        mangled += 'v';

    mangled += std::to_string(parameters.size()) + '_';
    for (auto &parameter : parameters)
        mangled += parameter.Info.Mangle();

    return mangled + result.Mangle();
}

llove::FunctionReference llove::Builder::GenFunction(const FunctionInfo &fn)
{
    const auto mangled = Mangle(
        fn.Interface,
        fn.Class,
        fn.Mutable,
        fn.Name,
        fn.Parameters,
        fn.Variadic.first,
        fn.Result);

    std::vector<Field> type_parameters;
    for (auto &parameter : fn.Parameters)
        type_parameters.emplace_back(parameter.Info);

    std::optional<Field> self;
    FunctionType::Ptr callee_type;

    if (fn.Class)
    {
        self = {
            .Mutable = fn.Mutable,
            .Reference = true,
            .Type = fn.Class,
        };
        callee_type = m_Context.GetFunction(std::move(type_parameters), fn.Variadic.first, fn.Result, *self);
    }
    else
    {
        callee_type = m_Context.GetFunction(std::move(type_parameters), fn.Variadic.first, fn.Result);
    }

    const auto callee = GetOrCreateFunction(mangled, callee_type, fn.Export || fn.Interface);

    auto function = fn.Register
                        ? PushFunction(fn.Expose, fn.Implicit, fn.Name, callee_type, callee)
                        : FunctionReference{
                            .Expose = fn.Expose,
                            .Implicit = fn.Implicit,
                            .Name = fn.Name,
                            .Type = callee_type,
                            .Callee = callee,
                        };

    if (!fn.Content)
        return function;

    Assert(callee->empty(), "function is already defined");

    m_DebugBuilder->BeginFunction(*this, fn.Name, fn.Loc, callee_type, mangled, callee);

    m_Parent = callee;
    m_Class = fn.Class;
    m_Result = fn.Result;

    const auto entry_block = CreateBlock("entry", callee);
    m_LLVMBuilder.SetInsertPoint(entry_block);

    m_DebugBuilder->EmitLoc(*this);
    PushFrame();
    GenParameters(callee, fn.Parameters, fn.Variadic, self);
    fn.Content->Gen(*this);
    PopFrame();

    m_DebugBuilder->EndFunction();

    for (auto &block : *callee)
    {
        if (block.getTerminator())
            continue;
        if (fn.Result.Type->IsVoid())
        {
            m_LLVMBuilder.SetInsertPoint(&block);
            m_LLVMBuilder.CreateRetVoid();
            continue;
        }
        Error("not all paths yield");
    }

    if (!verifyFunction(*callee, &llvm::errs()))
        return function;

    callee->print(llvm::errs());
    Error("function has errors");
}

void llove::Builder::GenParameters(
    llvm::Function *parent,
    const std::vector<Parameter> &parameters,
    const std::pair<bool, std::string> &variadic,
    const std::optional<Field> &self)
{
    auto iterator = parent->arg_begin();
    auto index = 1u;

    if (self)
    {
        const auto argument = iterator++;
        argument->setName("self");

        auto storage = Value::CreateL(self->Type, argument, self->Mutable);

        m_DebugBuilder->CreateParameter(*this, "self", index++, storage);
        SetValue("self", std::move(storage));
    }

    for (auto &parameter : parameters)
    {
        const auto argument = iterator++;
        argument->setName(parameter.Name);

        ValuePtr storage;
        if (parameter.Info.Reference)
        {
            storage = Value::CreateL(parameter.Info.Type, argument, parameter.Info.Mutable);
        }
        else if (parameter.Info.Type->IsClass())
        {
            const auto pointer = CreateAlloca(parameter.Info.Type->GenIR(*this), parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(parameter.Info.Type, pointer, parameter.Info.Mutable);

            auto class_type = As<ClassType>(parameter.Info.Type);
            if (const auto destructor = class_type->GetDestructor())
            {
                const auto function = GenFunction(
                    {
                        .Class = std::move(class_type),
                        .Mutable = destructor->Mutable,
                        .Expose = destructor->Expose,
                        .Name = destructor->Name,
                        .Result = destructor->Result,
                    });

                PushDestructor(pointer, function);
            }
        }
        else if (parameter.Info.Mutable)
        {
            const auto pointer = CreateAlloca(parameter.Info.Type->GenIR(*this), parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(parameter.Info.Type, pointer, parameter.Info.Mutable);
        }
        else
        {
            storage = Value::CreateR(parameter.Info.Type, argument);
        }

        m_DebugBuilder->CreateParameter(*this, parameter.Name, index++, storage);
        SetValue(parameter.Name, std::move(storage));
    }

    if (variadic.first && !variadic.second.empty())
    {
        const auto argument = iterator;
        argument->setName(variadic.second);

        const auto pointer = CreateAlloca(GetVariadicType(), parent);
        CreateStore(argument, pointer);

        auto storage = Value::CreateL(m_Context.GetVariadic(), pointer, true);

        m_DebugBuilder->CreateParameter(*this, variadic.second, index, storage);
        SetValue(variadic.second, std::move(storage));
    }
}
