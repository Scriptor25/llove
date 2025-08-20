#include <fstream>
#include <iostream>
#include <istream>
#include <ranges>
#include <cli/arguments.hpp>
#include <cli/table.hpp>
#include <cli/templates.hpp>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/stream.hpp>
#include <llove/tree.hpp>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/TargetParser/Host.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif

// --format=<"asm"|"obj">
// --output=<string|"stdout">
// --include=<string,...>...
// --print=<"llove"|"llvm",...>
// --print-output=<string|"stdout"|"stderr">
// --target=<string>
// --cpu=<string>
// --features=<string,...>
// --relocation=<"static"|"pic"|"dynamic-no-pic"|"ropi"|"rwpi"|"ropi-rwpi">
// --level=<"0"|"1"|"2"|"3"|"s"|"z">

// --option-binutils-version=<major,minor>
// --option-unsafe-fp-math
// --option-no-infs-fp-math
// --option-no-nans-fp-math
// --option-no-trapping-fp-math
// --option-no-signed-zeros-fp-math
// --option-approx-func-fp-math
// --option-enable-aix-extended-altivec-abi
// --option-honor-sign-dependent-rounding-fp-math
// --option-no-zeros-in-bss
// --option-guaranteed-tail-call-opt
// --option-stack-symbol-ordering
// --option-enable-fast-isel
// --option-enable-global-isel
// --option-global-isel-abort=<"enable"|"disable"|"disable-with-diag">
// --option-swift-async-frame-pointer=<"deployment-based"|"always"|"never">
// --option-use-init-array
// --option-disable-integrated-as
// --option-compress-debug-sections=<"none"|"zlib"|"zstd">
// --option-relax-elf-relocations
// --option-function-sections
// --option-data-sections
// --option-ignore-xcoff-visibility
// --option-xcoff-traceback-table
// --option-unique-section-names
// --option-unique-basic-block-section-names
// --option-trap-unreachable
// --option-no-trap-after-noreturn
// --option-tls-size
// --option-emulated-tls
// --option-enable-tls-desc
// --option-enable-ipra
// --option-emit-stack-size-section
// --option-enable-machine-outliner
// --option-enable-machine-function-splitter
// --option-supports-default-outlining
// --option-emit-addrsig
// --option-bb-sections=<"all"|"list"|"labels"|"present"|"none">
// --option-emit-call-site-info
// --option-supports-debug-entry-values
// --option-enable-debug-entry-values
// --option-value-tracking-variable-locations
// --option-force-dwarf-frame-section
// --option-xray-function-index
// --option-debug-strict-dwarf
// --option-hotpatch
// --option-ppc-gen-scalar-mass-entries
// --option-jmc-instrument
// --option-enable-cfi-fixup
// --option-mis-expect
// --option-xcoff-read-only-pointers
// --option-stack-usage-output
// --option-loop-alignment
// --option-float-abi-type=<"default"|"soft"|"hard">
// --option-allow-fp-op-fusion=<"fast"|"standard"|"strict">
// --option-thread-model=<"posix"|"single">
// --option-eabi-version=<"unknown"|"default"|"eabi4"|"eabi5"|"gnu">
// --option-debugger-tuning=<"default"|"gdb"|"lldb"|"sce"|"dbx">
// --option-fp-denormal-mode=<"ieee"|"preserve-sign"|"positive-zero"|"dynamic">
// --option-fp32-denormal-mode=<"ieee"|"preserve-sign"|"positive-zero"|"dynamic">
// --option-exception-model=<"none"|"dwarf-cfi"|"sjlj"|"arm"|"win-eh"|"wasm"|"aix"|"zos">

// --mc-option-relax-all
// --mc-option-no-exec-stack
// --mc-option-fatal-warnings
// --mc-option-no-warn
// --mc-option-no-deprecated-warn
// --mc-option-no-type-check
// --mc-option-save-temp-labels
// --mc-option-incremental-linker-compatible
// --mc-option-show-mc-encoding
// --mc-option-show-mc-inst
// --mc-option-asm-verbose
// --mc-option-preserve-asm-comments
// --mc-option-dwarf64
// --mc-option-emit-dwarf-unwind=<"always"|"no-compact-unwind"|"default">
// --mc-option-dwarf-version=<version>
// --mc-option-use-dwarf-directory=<"disable"|"enable"|"default">
// --mc-option-abi-name=<string>
// --mc-option-assembly-language=<string>
// --mc-option-split-dwarf-file=<string>
// --mc-option-as-secure-log-file=<string>
// --mc-option-emit-compact-unwind-non-canonical
// --mc-option-ppc-use-full-register-names

static void print_version()
{
    std::cerr << "llove v0.0.0" << std::endl;
}

static unsigned get_console_width()
{
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info);
    return info.srWindow.Right - info.srWindow.Left + 1;
#elif defined(__linux__)
    winsize size{};
    ioctl(fileno(stderr), TIOCGWINSZ, &size);
    return size.ws_col;
#endif
}

static void print_help(const std::map<std::string, cli::OptionTemplate> &templates, const bool ascii)
{
    print_version();

    std::cerr
            << std::endl
            << "USAGE" << std::endl
            << " llove <PATTERN{=<FILTER>},...> <FILENAME>" << std::endl
            << std::endl
            << "FILENAME: empty, \"stdin\" or existing filename" << std::endl
            << std::endl
            << "OPTIONS" << std::endl;

    const auto console_width = get_console_width();
    cli::Table table(std::cerr, 3, console_width ? console_width : 120u, ascii);

    table << "PATTERN" << "FILTER" << "DESCRIPTION";

    for (auto &template_ : templates | std::views::values)
    {
        std::string pattern_str;
        for (auto p = template_.Pattern.begin(); p != template_.Pattern.end(); ++p)
        {
            if (p != template_.Pattern.begin())
                pattern_str += ", ";
            pattern_str += *p;
        }
        table << pattern_str;

        std::string filter_str;
        if (template_.Type != cli::OptionTemplateType_Flag)
        {
            template_.Filter->Stringify(filter_str);
            if (template_.Type == cli::OptionTemplateType_Array)
                filter_str += ",...";
        }
        table << filter_str;

        table << template_.Description;
    }
}

int main(const int argc, const char *const *argv) try
{
    if (argc == 1)
    {
        std::cerr << "no arguments. use '--help', '-h', '-?' or '?' for help." << std::endl;
        return 1;
    }

    cli::Arguments arguments(argv + 1, argv + argc);

    if (arguments.flag("help"))
    {
        print_help(arguments.templates(), arguments.flag("ascii"));
        return 0;
    }

    if (arguments.flag("version"))
    {
        print_version();
        if (arguments.has_none_except({ "version" }))
            return 0;
    }

    auto input_filename = arguments.filename().empty() ? "stdin" : arguments.filename();

    llove::stream_ref<std::istream> input_stream_ref;
    if (input_filename == "stdin")
        input_stream_ref = llove::stream_ref(&std::cin, false);
    else
        input_stream_ref = llove::stream_ref<std::ifstream>(input_filename);

    if (input_stream_ref->fail())
    {
        std::cerr << "failed to open file '" << input_filename << "'" << std::endl;
        return 1;
    }

    auto debug = arguments.flag("debug");
    auto optimized = arguments.has_value_and_is_not("level", "0");
    auto profiling = arguments.flag("profiling");

    auto emission = llvm::DICompileUnit::DebugEmissionKind::FullDebug;
    if (std::string value; arguments.value("debug-kind", value))
    {
        static const std::map<std::string_view, llvm::DICompileUnit::DebugEmissionKind> VALUES
        {
            { "no-debug", llvm::DICompileUnit::NoDebug },
            { "full-debug", llvm::DICompileUnit::FullDebug },
            { "line-tables-only", llvm::DICompileUnit::LineTablesOnly },
            { "debug-directives-only", llvm::DICompileUnit::DebugDirectivesOnly },
        };
        emission = VALUES.at(value);
    }

    std::string debug_filename;
    (void) arguments.value("debug-output", debug_filename);

    llove::Context context;
    llove::Builder builder(
        context,
        debug,
        optimized,
        profiling,
        emission,
        input_filename,
        debug_filename,
        arguments.BuildCommandLine());
    llove::Parser parser(context, *input_stream_ref, input_filename);

    std::string print_filename, output_filename;
    llove::stream_ref<std::ostream> print_stream_ref, output_stream_ref;

    auto has_print_filename = arguments.value("print-output", print_filename);
    auto has_output_filename = arguments.value("output", output_filename);

    if (!has_print_filename || print_filename == "stderr")
        print_stream_ref = llove::stream_ref(&std::cerr, false);
    else if (print_filename == "stdout")
        print_stream_ref = llove::stream_ref(&std::cout, false);
    else
        print_stream_ref = llove::stream_ref<std::ofstream>(print_filename);

    if (print_stream_ref->fail())
    {
        std::cerr << "failed to open file '" << print_filename << "'" << std::endl;
        return 1;
    }

    auto print_llove = false, print_llvm = false;

    if (std::vector<std::string> print; arguments.array("print", print))
    {
        std::set print_set(print.begin(), print.end());
        print_llove = print_set.contains("llove");
        print_llvm = print_set.contains("llvm");
    }

    while (parser.Ok())
    {
        auto ptr = parser.Parse();
        if (print_llove)
            *print_stream_ref << ptr << std::endl;

        context.InstantiateReflections(builder);

        ptr->Gen(builder);
    }

    if (!has_output_filename || output_filename == "stdout")
        output_stream_ref = llove::stream_ref(&std::cout, false);
    else if (output_filename == "stderr")
        output_stream_ref = llove::stream_ref(&std::cerr, false);
    else
        output_stream_ref = llove::stream_ref<std::ofstream>(
            output_filename,
            std::ios_base::out | std::ios_base::binary);

    if (output_stream_ref->fail())
    {
        std::cerr << "failed to open file '" << output_filename << "'" << std::endl;
        return 1;
    }

    llove::SealInfo seal_info
    {
        .Print = print_llvm,
        .PrintStream = print_stream_ref.get(),
        .OutputStream = output_stream_ref.get(),
        .Format = llvm::CodeGenFileType::ObjectFile,
        .Triple = llvm::sys::getDefaultTargetTriple(),
        .CPU = "generic",
        .Features = {},
        .Options = {},
        .Relocation = llvm::Reloc::PIC_,
        .Level = llvm::OptimizationLevel::O0,
    };

    if (std::string value; arguments.value("format", value))
    {
        static const std::map<std::string_view, llvm::CodeGenFileType> VALUES
        {
            { "asm", llvm::CodeGenFileType::AssemblyFile },
            { "obj", llvm::CodeGenFileType::ObjectFile },
        };
        seal_info.Format = VALUES.at(value);
    }

    (void) arguments.value("triple", seal_info.Triple);
    (void) arguments.value("cpu", seal_info.CPU);
    (void) arguments.array("features", seal_info.Features);

    if (std::string value; arguments.value("relocation", value))
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
        seal_info.Relocation = VALUES.at(value);
    }

    if (std::string value; arguments.value("level", value))
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
        seal_info.Level = VALUES.at(value);
    }

    if (std::vector<std::string> version; arguments.array("option-binutils-version", version))
        seal_info.Options.BinutilsVersion = { std::stoi(version[0]), std::stoi(version[1]) };
    seal_info.Options.UnsafeFPMath = arguments.flag("option-unsafe-fp-math");
    seal_info.Options.NoInfsFPMath = arguments.flag("option-no-infs-fp-math");
    seal_info.Options.NoNaNsFPMath = arguments.flag("option-no-nans-fp-math");
    seal_info.Options.NoTrappingFPMath = arguments.flag("option-no-trapping-fp-math");
    seal_info.Options.NoSignedZerosFPMath = arguments.flag("option-no-signed-zeros-fp-math");
    seal_info.Options.ApproxFuncFPMath = arguments.flag("option-approx-func-fp-math");
    seal_info.Options.EnableAIXExtendedAltivecABI = arguments.flag("option-enable-aix-extended-altivec-abi");
    seal_info.Options.HonorSignDependentRoundingFPMathOption = arguments.flag(
        "option-honor-sign-dependent-rounding-fp-math");
    seal_info.Options.NoZerosInBSS = arguments.flag("option-no-zeros-in-bss");
    seal_info.Options.GuaranteedTailCallOpt = arguments.flag("option-guaranteed-tail-call-opt");
    seal_info.Options.StackSymbolOrdering = arguments.flag("option-stack-symbol-ordering");
    seal_info.Options.EnableFastISel = arguments.flag("option-enable-fast-isel");
    seal_info.Options.EnableGlobalISel = arguments.flag("option-enable-global-isel");
    if (std::string value; arguments.value("option-global-isel-abort", value))
    {
        static const std::map<std::string_view, llvm::GlobalISelAbortMode> VALUES
        {
            { "enable", llvm::GlobalISelAbortMode::Enable },
            { "disable", llvm::GlobalISelAbortMode::Disable },
            { "disable-with-diag", llvm::GlobalISelAbortMode::DisableWithDiag },
        };
        seal_info.Options.GlobalISelAbort = VALUES.at(value);
    }
    if (std::string value; arguments.value("option-swift-async-frame-pointer", value))
    {
        static const std::map<std::string_view, llvm::SwiftAsyncFramePointerMode> VALUES
        {
            { "deployment-based", llvm::SwiftAsyncFramePointerMode::DeploymentBased },
            { "always", llvm::SwiftAsyncFramePointerMode::Always },
            { "never", llvm::SwiftAsyncFramePointerMode::Never },
        };
        seal_info.Options.SwiftAsyncFramePointer = VALUES.at(value);
    }
    seal_info.Options.UseInitArray = arguments.flag("option-use-init-array");
    seal_info.Options.DisableIntegratedAS = arguments.flag("option-disable-integrated-as");
    if (std::string value; arguments.value("option-compress-debug-sections", value))
    {
        static const std::map<std::string_view, llvm::DebugCompressionType> VALUES
        {
            { "none", llvm::DebugCompressionType::None },
            { "zlib", llvm::DebugCompressionType::Zlib },
            { "zstd", llvm::DebugCompressionType::Zstd },
        };
        seal_info.Options.CompressDebugSections = VALUES.at(value);
    }
    seal_info.Options.RelaxELFRelocations = arguments.flag("option-relax-elf-relocations");
    seal_info.Options.FunctionSections = arguments.flag("option-function-sections");
    seal_info.Options.DataSections = arguments.flag("option-data-sections");
    seal_info.Options.IgnoreXCOFFVisibility = arguments.flag("option-ignore-xcoff-visibility");
    seal_info.Options.XCOFFTracebackTable = arguments.flag("option-xcoff-traceback-table");
    seal_info.Options.UniqueSectionNames = arguments.flag("option-unique-section-names");
    seal_info.Options.UniqueBasicBlockSectionNames = arguments.flag("option-unique-basic-block-section-names");
    seal_info.Options.TrapUnreachable = arguments.flag("option-trap-unreachable");
    seal_info.Options.NoTrapAfterNoreturn = arguments.flag("option-no-trap-after-noreturn");
    seal_info.Options.TLSSize = arguments.flag("option-tls-size");
    seal_info.Options.EmulatedTLS = arguments.flag("option-emulated-tls");
    seal_info.Options.EnableTLSDESC = arguments.flag("option-enable-tls-desc");
    seal_info.Options.EnableIPRA = arguments.flag("option-enable-ipra");
    seal_info.Options.EmitStackSizeSection = arguments.flag("option-emit-stack-size-section");
    seal_info.Options.EnableMachineOutliner = arguments.flag("option-enable-machine-outliner");
    seal_info.Options.EnableMachineFunctionSplitter = arguments.flag("option-enable-machine-function-splitter");
    seal_info.Options.SupportsDefaultOutlining = arguments.flag("option-supports-default-outlining");
    seal_info.Options.EmitAddrsig = arguments.flag("option-emit-addrsig");
    if (std::string value; arguments.value("option-bb-sections", value))
    {
        static const std::map<std::string_view, llvm::BasicBlockSection> VALUES
        {
            { "all", llvm::BasicBlockSection::All },
            { "list", llvm::BasicBlockSection::List },
            { "labels", llvm::BasicBlockSection::Labels },
            { "preset", llvm::BasicBlockSection::Preset },
            { "none", llvm::BasicBlockSection::None },
        };
        seal_info.Options.BBSections = VALUES.at(value);
    }
    seal_info.Options.EmitCallSiteInfo = arguments.flag("option-emit-call-site-info");
    seal_info.Options.SupportsDebugEntryValues = arguments.flag("option-supports-debug-entry-values");
    seal_info.Options.EnableDebugEntryValues = arguments.flag("option-enable-debug-entry-values");
    seal_info.Options.ValueTrackingVariableLocations = arguments.flag("option-value-tracking-variable-locations");
    seal_info.Options.ForceDwarfFrameSection = arguments.flag("option-force-dwarf-frame-section");
    seal_info.Options.XRayFunctionIndex = arguments.flag("option-xray-function-index");
    seal_info.Options.DebugStrictDwarf = arguments.flag("option-debug-strict-dwarf");
    seal_info.Options.Hotpatch = arguments.flag("option-hotpatch");
    seal_info.Options.PPCGenScalarMASSEntries = arguments.flag("option-ppc-gen-scalar-mass-entries");
    seal_info.Options.JMCInstrument = arguments.flag("option-jmc-instrument");
    seal_info.Options.EnableCFIFixup = arguments.flag("option-enable-cfi-fixup");
    seal_info.Options.MisExpect = arguments.flag("option-mis-expect");
    seal_info.Options.XCOFFReadOnlyPointers = arguments.flag("option-xcoff-read-only-pointers");
    (void) arguments.value("option-stack-usage-output", seal_info.Options.StackUsageOutput);
    seal_info.Options.LoopAlignment = arguments.flag("option-loop-alignment");
    if (std::string value; arguments.value("option-float-abi-type", value))
    {
        static const std::map<std::string_view, llvm::FloatABI::ABIType> VALUES
        {
            { "default", llvm::FloatABI::Default },
            { "soft", llvm::FloatABI::Soft },
            { "hard", llvm::FloatABI::Hard },
        };
        seal_info.Options.FloatABIType = VALUES.at(value);
    }
    if (std::string value; arguments.value("option-allow-fp-op-fusion", value))
    {
        static const std::map<std::string_view, llvm::FPOpFusion::FPOpFusionMode> VALUES
        {
            { "fast", llvm::FPOpFusion::Fast },
            { "standard", llvm::FPOpFusion::Standard },
            { "strict", llvm::FPOpFusion::Strict },
        };
        seal_info.Options.AllowFPOpFusion = VALUES.at(value);
    }
    if (std::string value; arguments.value("option-thread-model", value))
    {
        static const std::map<std::string_view, llvm::ThreadModel::Model> VALUES
        {
            { "posix", llvm::ThreadModel::POSIX },
            { "single", llvm::ThreadModel::Single },
        };
        seal_info.Options.ThreadModel = VALUES.at(value);
    }
    if (std::string value; arguments.value("option-eabi-version", value))
    {
        static const std::map<std::string_view, llvm::EABI> VALUES
        {
            { "unknown", llvm::EABI::Unknown },
            { "default", llvm::EABI::Default },
            { "eabi4", llvm::EABI::EABI4 },
            { "eabi5", llvm::EABI::EABI5 },
            { "gnu", llvm::EABI::GNU },
        };
        seal_info.Options.EABIVersion = VALUES.at(value);
    }
    if (std::string value; arguments.value("option-debugger-tuning", value))
    {
        static const std::map<std::string_view, llvm::DebuggerKind> VALUES
        {
            { "default", llvm::DebuggerKind::Default },
            { "gdb", llvm::DebuggerKind::GDB },
            { "lldb", llvm::DebuggerKind::LLDB },
            { "sce", llvm::DebuggerKind::SCE },
            { "dbx", llvm::DebuggerKind::DBX },
        };
        seal_info.Options.DebuggerTuning = VALUES.at(value);
    }
    if (std::vector<std::string> values; arguments.array("option-fp-denormal-mode", values))
    {
        static const std::map<std::string_view, llvm::DenormalMode::DenormalModeKind> VALUES
        {
            { "ieee", llvm::DenormalMode::IEEE },
            { "preserve-sign", llvm::DenormalMode::PreserveSign },
            { "positive-zero", llvm::DenormalMode::PositiveZero },
            { "dynamic", llvm::DenormalMode::Dynamic },
        };
        seal_info.Options.setFPDenormalMode({ VALUES.at(values[0]), VALUES.at(values[1]) });
    }
    if (std::vector<std::string> values; arguments.array("option-fp32-denormal-mode", values))
    {
        static const std::map<std::string_view, llvm::DenormalMode::DenormalModeKind> VALUES
        {
            { "ieee", llvm::DenormalMode::IEEE },
            { "preserve-sign", llvm::DenormalMode::PreserveSign },
            { "positive-zero", llvm::DenormalMode::PositiveZero },
            { "dynamic", llvm::DenormalMode::Dynamic },
        };
        seal_info.Options.setFP32DenormalMode({ VALUES.at(values[0]), VALUES.at(values[1]) });
    }
    if (std::string value; arguments.value("option-exception-model", value))
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
        seal_info.Options.ExceptionModel = VALUES.at(value);
    }

    seal_info.Options.MCOptions.MCRelaxAll = arguments.flag("mc-option-relax-all");
    seal_info.Options.MCOptions.MCNoExecStack = arguments.flag("mc-option-no-exec-stack");
    seal_info.Options.MCOptions.MCFatalWarnings = arguments.flag("mc-option-fatal-warnings");
    seal_info.Options.MCOptions.MCNoWarn = arguments.flag("mc-option-no-warn");
    seal_info.Options.MCOptions.MCNoDeprecatedWarn = arguments.flag("mc-option-no-deprecated-warn");
    seal_info.Options.MCOptions.MCNoTypeCheck = arguments.flag("mc-option-no-type-check");
    seal_info.Options.MCOptions.MCSaveTempLabels = arguments.flag("mc-option-save-temp-labels");
    seal_info.Options.MCOptions.MCIncrementalLinkerCompatible = arguments.flag(
        "mc-option-incremental-linker-compatible");
    seal_info.Options.MCOptions.ShowMCEncoding = arguments.flag("mc-option-show-mc-encoding");
    seal_info.Options.MCOptions.ShowMCInst = arguments.flag("mc-option-show-mc-inst");
    seal_info.Options.MCOptions.AsmVerbose = arguments.flag("mc-option-asm-verbose");
    seal_info.Options.MCOptions.PreserveAsmComments = arguments.flag("mc-option-preserve-asm-comments");
    seal_info.Options.MCOptions.Dwarf64 = arguments.flag("mc-option-dwarf64");
    if (std::string value; arguments.value("mc-option-emit-dwarf-unwind", value))
    {
        static const std::map<std::string_view, llvm::EmitDwarfUnwindType> VALUES
        {
            { "always", llvm::EmitDwarfUnwindType::Always },
            { "no-compact-unwind", llvm::EmitDwarfUnwindType::NoCompactUnwind },
            { "default", llvm::EmitDwarfUnwindType::Default },
        };
        seal_info.Options.MCOptions.EmitDwarfUnwind = VALUES.at(value);
    }
    if (std::string value; arguments.value("mc-option-dwarf-version", value))
        seal_info.Options.MCOptions.DwarfVersion = std::stoi(value);
    if (std::string value; arguments.value("mc-option-use-dwarf-directory", value))
    {
        static const std::map<std::string_view, llvm::MCTargetOptions::DwarfDirectory> VALUES
        {
            { "disable", llvm::MCTargetOptions::DisableDwarfDirectory },
            { "enable", llvm::MCTargetOptions::EnableDwarfDirectory },
            { "default", llvm::MCTargetOptions::DefaultDwarfDirectory },
        };
        seal_info.Options.MCOptions.MCUseDwarfDirectory = VALUES.at(value);
    }
    (void) arguments.value("mc-option-abi-name", seal_info.Options.MCOptions.ABIName);
    (void) arguments.value("mc-option-assembly-language", seal_info.Options.MCOptions.AssemblyLanguage);
    (void) arguments.value("mc-option-split-dwarf-file", seal_info.Options.MCOptions.SplitDwarfFile);
    (void) arguments.value("mc-option-as-secure-log-file", seal_info.Options.MCOptions.AsSecureLogFile);
    seal_info.Options.MCOptions.EmitCompactUnwindNonCanonical = arguments.flag(
        "mc-option-emit-compact-unwind-non-canonical");
    seal_info.Options.MCOptions.PPCUseFullRegisterNames = arguments.flag("mc-option-ppc-use-full-register-names");

    builder.Seal(seal_info);

    llvm::llvm_shutdown();

    return 0;
}
catch (const llove::ref_exception<llove::ErrorStack> &cause)
{
    cause->Print(std::cerr);
    return 1;
}
