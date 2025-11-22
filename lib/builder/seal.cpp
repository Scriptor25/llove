#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/stream.hpp>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Target/TargetMachine.h>

void llove::Builder::Seal(
    bool print,
    std::ostream& print_stream,
    std::ostream& output_stream,
    llvm::CodeGenFileType code_gen_type,
    llvm::OptimizationLevel optimization_level)
{
    m_DebugBuilder.EndModule();

    if (llvm::verifyModule(m_LLVMModule, &llvm::errs()))
    {
        m_LLVMModule.print(llvm::errs(), nullptr);
        Error("module has errors");
    }

    raw_pwrite_stream_adapter raw_print_stream(print_stream);
    raw_pwrite_stream_adapter raw_output_stream(output_stream);

    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    llvm::PassInstrumentationCallbacks pic;
    llvm::StandardInstrumentations si(m_LLVMContext, true);

    si.registerCallbacks(pic, &mam);

    llvm::PassBuilder pb(m_TargetMachine.get());
    pb.registerLoopAnalyses(lam);
    pb.registerFunctionAnalyses(fam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerModuleAnalyses(mam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    auto mpm = pb.buildPerModuleDefaultPipeline(optimization_level);
    mpm.run(m_LLVMModule, mam);

    if (print)
    {
        m_LLVMModule.print(raw_print_stream, nullptr);
    }

    // TODO: pls tell llvm devs to update their codegen system!!!
    llvm::legacy::PassManager codegen_pass;
    const auto emit_error = m_TargetMachine->addPassesToEmitFile(codegen_pass, raw_output_stream, nullptr, code_gen_type);
    Assert(!emit_error, "target machine cannot emit specified codegen type");

    codegen_pass.run(m_LLVMModule);
}
