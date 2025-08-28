#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>
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
      m_LLVMBuilder(m_LLVMContext),
      m_LLVMModule(module_id, m_LLVMContext),
      m_Debug(debug),
      m_DebugBuilder(m_Debug, m_LLVMModule, source_path, optimized, profiling, command_line, emission)
{
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

    m_TargetMachine = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(
            target_triple,
            cpu,
            features,
            machine.Options,
            machine.Relocation
        ));
    Assert(
        m_TargetMachine != nullptr,
        "failed to create target machine for triple '{}', cpu '{}' and features '{}'",
        target_triple,
        cpu,
        features);

    m_LLVMModule.setSourceFileName(source_path.string());
    m_LLVMModule.setDataLayout(m_TargetMachine->createDataLayout());
    m_LLVMModule.setTargetTriple(target_triple);

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

llove::DebugBuilder &llove::Builder::GetDebug()
{
    return m_DebugBuilder;
}

void llove::Builder::EmitLoc(const Location &loc)
{
    m_DebugBuilder.EmitLoc(*this, loc);
}

llvm::LLVMContext &llove::Builder::GetLLVMContext()
{
    return m_LLVMContext;
}

const llvm::DataLayout &llove::Builder::GetDataLayout() const
{
    return m_LLVMModule.getDataLayout();
}
