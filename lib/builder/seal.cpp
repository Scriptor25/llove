#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>

class std_ostream_adapter final : public llvm::raw_pwrite_stream
{
public:
    explicit std_ostream_adapter(std::ostream &stream)
        : raw_pwrite_stream(true),
          m_Stream(stream)
    {
    }

    void write_impl(const char *ptr, const size_t size) override
    {
        m_Stream.write(ptr, static_cast<std::streamsize>(size));
    }

    [[nodiscard]] uint64_t current_pos() const override
    {
        return m_Stream.tellp();
    }

    void pwrite_impl(const char *ptr, const size_t size, const uint64_t offset) override
    {
        const auto current = m_Stream.tellp();
        m_Stream.seekp(static_cast<std::streamsize>(offset));
        m_Stream.write(ptr, static_cast<std::streamsize>(size));
        m_Stream.seekp(current);
    }

private:
    std::ostream &m_Stream;
};

void llove::Builder::Seal(const SealInfo &info)
{
    m_DIBuilder.finalize();

    Assert(!llvm::verifyModule(m_Module, &llvm::errs()), "module has errors");

    std_ostream_adapter print_stream(*info.PrintStream);
    std_ostream_adapter output_stream(*info.OutputStream);

    if (info.Print)
        m_Module.print(print_stream, nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto target_triple = info.Triple.empty() ? llvm::sys::getDefaultTargetTriple() : info.Triple;
    auto cpu = info.CPU.empty() ? "generic" : info.CPU;

    std::string features;
    for (auto i = info.Features.begin(); i != info.Features.end(); ++i)
    {
        if (i != info.Features.begin())
            features += ',';
        features += *i;
    }

    std::string target_error;
    const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);
    Assert(target != nullptr, "failed to get target for triple '{}': {}", target_triple, target_error);

    const auto target_machine = target->createTargetMachine(
        target_triple,
        cpu,
        features,
        info.Options,
        info.Relocation);
    Assert(
        target_machine != nullptr,
        "failed to create target machine for triple '{}', cpu '{}' and features '{}'",
        target_triple,
        cpu,
        features);

    m_Module.setDataLayout(target_machine->createDataLayout());
    m_Module.setTargetTriple(target_triple);

    m_Module.print(llvm::errs(), nullptr);

    // llvm::PassBuilder pb(target_machine);
    // auto mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

    // llvm::ModuleAnalysisManager mam;
    // llvm::LoopAnalysisManager lam;
    // llvm::CGSCCAnalysisManager cgam;
    // llvm::FunctionAnalysisManager fam;

    // pb.registerLoopAnalyses(lam);
    // pb.registerFunctionAnalyses(fam);
    // pb.registerCGSCCAnalyses(cgam);
    // pb.registerModuleAnalyses(mam);
    // pb.crossRegisterProxies(lam, fam, cgam, mam);

    // mpm.run(m_Module, mam);

    // TODO: pls tell llvm devs to update their codegen system!!!
    llvm::legacy::PassManager codegen_pass;
    const auto emit_error = target_machine->addPassesToEmitFile(
        codegen_pass,
        output_stream,
        nullptr,
        info.Format);
    Assert(!emit_error, "target machine cannot emit specified codegen type");

    codegen_pass.run(m_Module);
}
