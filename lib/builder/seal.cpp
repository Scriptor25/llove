#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/stream.hpp>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>

void llove::Builder::Seal(const SealInfo &info)
{
    m_DebugBuilder->EndModule();

    Assert(!llvm::verifyModule(m_LLVMModule, &llvm::errs()), "module has errors");

    raw_pwrite_stream_adapter print_stream(*info.PrintStream);
    raw_pwrite_stream_adapter output_stream(*info.OutputStream);

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

    const auto target_machine = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(
            target_triple,
            cpu,
            features,
            info.Options,
            info.Relocation));
    Assert(
        target_machine != nullptr,
        "failed to create target machine for triple '{}', cpu '{}' and features '{}'",
        target_triple,
        cpu,
        features);

    m_LLVMModule.setDataLayout(target_machine->createDataLayout());
    m_LLVMModule.setTargetTriple(target_triple);

    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    llvm::PassInstrumentationCallbacks pic;
    llvm::StandardInstrumentations si(m_LLVMContext, true);

    si.registerCallbacks(pic, &mam);

    llvm::PassBuilder pb(target_machine.get());
    pb.registerLoopAnalyses(lam);
    pb.registerFunctionAnalyses(fam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerModuleAnalyses(mam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    auto mpm = pb.buildPerModuleDefaultPipeline(info.Level);
    mpm.run(m_LLVMModule, mam);

    if (info.Print)
        m_LLVMModule.print(print_stream, nullptr);

    // TODO: pls tell llvm devs to update their codegen system!!!
    llvm::legacy::PassManager codegen_pass;
    const auto emit_error = target_machine->addPassesToEmitFile(
        codegen_pass,
        output_stream,
        nullptr,
        info.Format);
    Assert(!emit_error, "target machine cannot emit specified codegen type");

    codegen_pass.run(m_LLVMModule);
}
