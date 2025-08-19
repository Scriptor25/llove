#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

llove::Builder::Builder(
    Context &context,
    const bool debug,
    const bool optimized,
    const bool profiling,
    const llvm::DICompileUnit::DebugEmissionKind emission,
    const std::filesystem::path &source_path,
    const std::filesystem::path &debug_path,
    const std::string &command_line)
    : Builder(
        context,
        debug,
        optimized,
        profiling,
        emission,
        source_path,
        debug_path,
        command_line,
        source_path.filename().replace_extension().string())
{
}

llove::Builder::Builder(
    Context &context,
    const bool debug,
    const bool optimized,
    const bool profiling,
    const llvm::DICompileUnit::DebugEmissionKind emission,
    const std::filesystem::path &source_path,
    const std::filesystem::path &debug_path,
    const std::string &command_line,
    const std::string &module_id)
    : m_Context(context),
      m_Debug(debug),
      m_LLVMBuilder(m_LLVMContext),
      m_LLVMModule(module_id, m_LLVMContext)
{
    m_LLVMModule.setSourceFileName(source_path.string());

    m_DebugBuilder = std::make_unique<DebugBuilder>(
        m_Debug,
        m_LLVMModule,
        source_path,
        debug_path,
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
        fn.VarArg.first,
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
        callee_type = m_Context.GetFunction(type_parameters, fn.VarArg.first, fn.Result, *self);
    }
    else
    {
        callee_type = m_Context.GetFunction(type_parameters, fn.VarArg.first, fn.Result);
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

    GenParameters(callee, fn.Parameters, self);

    if (fn.VarArg.first && !fn.VarArg.second.empty())
    {
        // TODO: target dependent
        const auto array_type = GetArrayType(GetVAListTagType(), 1);
        const auto pointer = CreateAlloca(array_type, callee);
        pointer->setAlignment(llvm::Align(16));
        const auto ap = m_LLVMBuilder.CreateConstInBoundsGEP2_64(array_type, pointer, 0, 0);
        CreateVAStart(ap);
        DeferAction(
            nullptr,
            [this, ap]
            {
                CreateVAEnd(ap);
            });
        SetValue(fn.VarArg.second, Value::CreateR(m_Context.GetPointer(false), ap));
    }

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

    Assert(!verifyFunction(*callee, &llvm::errs()), "function has errors");
    return function;
}

void llove::Builder::GenParameters(
    llvm::Function *parent,
    const std::vector<Parameter> &parameters,
    const std::optional<Field> &self)
{
    auto offset = 0u;
    if (self)
    {
        offset = 1u;

        const auto argument = parent->getArg(0);
        argument->setName("self");

        auto storage = Value::CreateL(self->Type, argument, self->Mutable);

        m_DebugBuilder->CreateParameter(*this, "self", 1u, storage);
        SetValue("self", std::move(storage));
    }

    for (unsigned i = 0; i < parent->arg_size() - offset; ++i)
    {
        auto &parameter = parameters.at(i);

        const auto argument = parent->getArg(i + offset);
        argument->setName(parameter.Name);

        ValuePtr storage;
        if (parameter.Info.Reference)
        {
            storage = Value::CreateL(parameter.Info.Type, argument, parameter.Info.Mutable);
        }
        else if (parameter.Info.Type->IsClass())
        {
            const auto pointer = CreateAlloca(parameter.Info.Type, parent);
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
            const auto pointer = CreateAlloca(parameter.Info.Type, parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(parameter.Info.Type, pointer, parameter.Info.Mutable);
        }
        else
        {
            storage = Value::CreateR(parameter.Info.Type, argument);
        }

        m_DebugBuilder->CreateParameter(*this, parameter.Name, i + offset + 1u, storage);
        SetValue(parameter.Name, storage);
    }
}
