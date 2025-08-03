#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

llove::Builder::Builder(
    Context &types,
    const bool debug,
    const bool optimized,
    const bool profiling,
    const llvm::DICompileUnit::DebugEmissionKind emission,
    const std::filesystem::path &source_path,
    const std::filesystem::path &debug_path,
    const std::string &command_line)
    : Builder(
        types,
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
    Context &types,
    const bool debug,
    const bool optimized,
    const bool profiling,
    const llvm::DICompileUnit::DebugEmissionKind emission,
    const std::filesystem::path &source_path,
    const std::filesystem::path &debug_path,
    const std::string &command_line,
    const std::string &module_id)
    : m_Types(types),
      m_Debug(debug),
      m_Builder(m_Context),
      m_Module(module_id, m_Context)
{
    m_Module.setSourceFileName(source_path.string());

    m_DebugBuilder = std::make_unique<DebugBuilder>(
        m_Debug,
        m_Module,
        source_path,
        debug_path,
        optimized,
        profiling,
        command_line,
        emission);
}

llove::Context &llove::Builder::GetTypes() const
{
    return m_Types;
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

llvm::LLVMContext &llove::Builder::GetContext()
{
    return m_Context;
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

llvm::Value *llove::Builder::CreateGlobalString(const std::string &value)
{
    return m_Builder.CreateGlobalStringPtr(value, {}, 0, &m_Module);
}

llove::FunctionReference &llove::Builder::GenFunction(const FunctionInfo &fn)
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
    for (auto &parameter : fn.Parameters)
        type_parameters.emplace_back(parameter.Info);

    std::optional<Field> self;
    FunctionType::Ptr function_type;

    if (fn.Class)
    {
        self = {
            .Mutable = fn.Mutable,
            .Reference = true,
            .Type = fn.Class,
        };
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result, *self);
    }
    else
    {
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result);
    }

    const auto function = GetOrCreateFunction(mangled, function_type, fn.Interface);
    auto &reference = PushFunction(fn.Expose, fn.Implicit, fn.Name, function_type, function);

    if (!fn.Content)
        return reference;

    Assert(function->empty(), "function is already defined");

    m_DebugBuilder->BeginFunction(*this, fn.Name, fn.Loc, function_type, mangled, function);

    m_Parent = function;
    m_Class = fn.Class;
    m_Result = fn.Result;

    const auto entry_block = CreateBlock("entry", function);
    m_Builder.SetInsertPoint(entry_block);

    m_DebugBuilder->EmitLoc(*this);
    PushFrame();
    GenParameters(function, fn.Parameters, self);
    fn.Content->Gen(*this);
    PopFrame();

    m_DebugBuilder->EndFunction();

    for (auto &block : *function)
    {
        if (block.getTerminator())
            continue;
        if (fn.Result.Type->IsVoid())
        {
            m_Builder.SetInsertPoint(&block);
            m_Builder.CreateRetVoid();
            continue;
        }
        Error("not all paths yield");
    }

    Assert(!verifyFunction(*function, &llvm::errs()), "function has errors");
    return reference;
}

void llove::Builder::GenParameters(
    llvm::Function *function,
    const std::vector<Parameter> &parameters,
    const std::optional<Field> &self)
{
    auto offset = 0u;
    if (self)
    {
        offset = 1u;

        const auto argument = function->getArg(0);
        argument->setName("self");

        auto storage = Value::CreateL(self->Type, argument, self->Mutable);

        m_DebugBuilder->CreateParameter(*this, "self", 1u, storage);
        SetValue("self", std::move(storage));
    }

    for (unsigned i = 0; i < function->arg_size() - offset; ++i)
    {
        auto &parameter = parameters.at(i);

        const auto argument = function->getArg(i + offset);
        argument->setName(parameter.Name);

        ValuePtr storage;
        if (parameter.Info.Reference)
        {
            storage = Value::CreateL(parameter.Info.Type, argument, parameter.Info.Mutable);
        }
        else if (parameter.Info.Type->IsClass())
        {
            const auto pointer = CreateAlloca(parameter.Info.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(parameter.Info.Type, pointer, parameter.Info.Mutable);

            auto class_type = As<ClassType>(parameter.Info.Type);
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

                PushDestructor(pointer, reference);
            }
        }
        else if (parameter.Info.Mutable)
        {
            const auto pointer = CreateAlloca(parameter.Info.Type, function);
            m_Builder.CreateStore(argument, pointer);

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
