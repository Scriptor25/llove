#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

void llove::Builder::Seal(const std::string &filename)
{
    m_Module.print(llvm::outs(), nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string target_error;

    auto target_triple = llvm::sys::getDefaultTargetTriple();
    const auto target = llvm::TargetRegistry::lookupTarget(target_triple, target_error);

    Assert(target != nullptr, "failed to get target for triple '{}': {}", target_triple, target_error);

    const std::string cpu = "generic";
    const std::string features;
    const llvm::TargetOptions options;

    const auto target_machine = target->createTargetMachine(target_triple, cpu, features, options, llvm::Reloc::PIC_);

    m_Module.setDataLayout(target_machine->createDataLayout());
    m_Module.setTargetTriple(target_triple);

    std::error_code error_code;
    llvm::raw_fd_ostream stream(filename, error_code, llvm::sys::fs::OF_None);

    Assert(!error_code, "failed to open file '{}': {}", filename, error_code.message());

    llvm::legacy::PassManager pass_manager;
    const auto emit_error = target_machine->addPassesToEmitFile(
        pass_manager,
        stream,
        nullptr,
        llvm::CodeGenFileType::ObjectFile);
    Assert(!emit_error, "target machine cannot emit specified file type");

    pass_manager.run(m_Module);
    stream.flush();
    stream.close();
}
