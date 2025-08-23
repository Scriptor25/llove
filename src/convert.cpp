#include <filesystem>
#include <cli/arguments.hpp>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetOptions.h>

template<>
bool cli::convert_value(int &dst, const std::string &value)
{
    dst = std::stoi(value);
    return true;
}

template<>
bool cli::convert_value(std::filesystem::path &dst, const std::string &value)
{
    dst = std::filesystem::weakly_canonical(value);
    return true;
}

template<>
bool cli::convert_value(llvm::DICompileUnit::DebugEmissionKind &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::DICompileUnit::DebugEmissionKind> VALUES
    {
        { "no-debug", llvm::DICompileUnit::NoDebug },
        { "full-debug", llvm::DICompileUnit::FullDebug },
        { "line-tables-only", llvm::DICompileUnit::LineTablesOnly },
        { "debug-directives-only", llvm::DICompileUnit::DebugDirectivesOnly },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::CodeGenFileType &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::CodeGenFileType> VALUES
    {
        { "asm", llvm::CodeGenFileType::AssemblyFile },
        { "obj", llvm::CodeGenFileType::ObjectFile },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::Reloc::Model &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::Reloc::Model> VALUES
    {
        { "static", llvm::Reloc::Static },
        { "pic", llvm::Reloc::PIC_ },
        { "dynamic-no-pic", llvm::Reloc::DynamicNoPIC },
        { "ropi", llvm::Reloc::ROPI },
        { "rwpi", llvm::Reloc::RWPI },
        { "ropi-rwpi", llvm::Reloc::ROPI_RWPI },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::OptimizationLevel &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::OptimizationLevel> VALUES
    {
        { "0", llvm::OptimizationLevel::O0 },
        { "1", llvm::OptimizationLevel::O1 },
        { "2", llvm::OptimizationLevel::O2 },
        { "3", llvm::OptimizationLevel::O3 },
        { "s", llvm::OptimizationLevel::Os },
        { "z", llvm::OptimizationLevel::Oz },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::GlobalISelAbortMode &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::GlobalISelAbortMode> VALUES
    {
        { "enable", llvm::GlobalISelAbortMode::Enable },
        { "disable", llvm::GlobalISelAbortMode::Disable },
        { "disable-with-diag", llvm::GlobalISelAbortMode::DisableWithDiag },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::SwiftAsyncFramePointerMode &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::SwiftAsyncFramePointerMode> VALUES
    {
        { "deployment-based", llvm::SwiftAsyncFramePointerMode::DeploymentBased },
        { "always", llvm::SwiftAsyncFramePointerMode::Always },
        { "never", llvm::SwiftAsyncFramePointerMode::Never },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::DebugCompressionType &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::DebugCompressionType> VALUES
    {
        { "none", llvm::DebugCompressionType::None },
        { "zlib", llvm::DebugCompressionType::Zlib },
        { "zstd", llvm::DebugCompressionType::Zstd },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::BasicBlockSection &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::BasicBlockSection> VALUES
    {
        { "all", llvm::BasicBlockSection::All },
        { "list", llvm::BasicBlockSection::List },
        { "labels", llvm::BasicBlockSection::Labels },
        { "preset", llvm::BasicBlockSection::Preset },
        { "none", llvm::BasicBlockSection::None },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::FloatABI::ABIType &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::FloatABI::ABIType> VALUES
    {
        { "default", llvm::FloatABI::Default },
        { "soft", llvm::FloatABI::Soft },
        { "hard", llvm::FloatABI::Hard },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::FPOpFusion::FPOpFusionMode &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::FPOpFusion::FPOpFusionMode> VALUES
    {
        { "fast", llvm::FPOpFusion::Fast },
        { "standard", llvm::FPOpFusion::Standard },
        { "strict", llvm::FPOpFusion::Strict },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::ThreadModel::Model &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::ThreadModel::Model> VALUES
    {
        { "posix", llvm::ThreadModel::POSIX },
        { "single", llvm::ThreadModel::Single },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::EABI &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::EABI> VALUES
    {
        { "unknown", llvm::EABI::Unknown },
        { "default", llvm::EABI::Default },
        { "eabi4", llvm::EABI::EABI4 },
        { "eabi5", llvm::EABI::EABI5 },
        { "gnu", llvm::EABI::GNU },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::DebuggerKind &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::DebuggerKind> VALUES
    {
        { "default", llvm::DebuggerKind::Default },
        { "gdb", llvm::DebuggerKind::GDB },
        { "lldb", llvm::DebuggerKind::LLDB },
        { "sce", llvm::DebuggerKind::SCE },
        { "dbx", llvm::DebuggerKind::DBX },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::DenormalMode::DenormalModeKind &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::DenormalMode::DenormalModeKind> VALUES
    {
        { "ieee", llvm::DenormalMode::IEEE },
        { "preserve-sign", llvm::DenormalMode::PreserveSign },
        { "positive-zero", llvm::DenormalMode::PositiveZero },
        { "dynamic", llvm::DenormalMode::Dynamic },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::ExceptionHandling &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::ExceptionHandling> VALUES
    {
        { "none", llvm::ExceptionHandling::None },
        { "dwarf-cfi", llvm::ExceptionHandling::DwarfCFI },
        { "sjlj", llvm::ExceptionHandling::SjLj },
        { "arm", llvm::ExceptionHandling::ARM },
        { "win-eh", llvm::ExceptionHandling::WinEH },
        { "wasm", llvm::ExceptionHandling::Wasm },
        { "aix", llvm::ExceptionHandling::AIX },
        { "zos", llvm::ExceptionHandling::ZOS },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::EmitDwarfUnwindType &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::EmitDwarfUnwindType> VALUES
    {
        { "always", llvm::EmitDwarfUnwindType::Always },
        { "no-compact-unwind", llvm::EmitDwarfUnwindType::NoCompactUnwind },
        { "default", llvm::EmitDwarfUnwindType::Default },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}

template<>
bool cli::convert_value(llvm::MCTargetOptions::DwarfDirectory &dst, const std::string &value)
{
    static const std::map<std::string_view, llvm::MCTargetOptions::DwarfDirectory> VALUES
    {
        { "disable", llvm::MCTargetOptions::DisableDwarfDirectory },
        { "enable", llvm::MCTargetOptions::EnableDwarfDirectory },
        { "default", llvm::MCTargetOptions::DefaultDwarfDirectory },
    };

    if (!VALUES.contains(value))
        return false;

    dst = VALUES.at(value);
    return true;
}
