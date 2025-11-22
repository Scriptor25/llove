#include <cli/arguments.hpp>
#include <cli/table.hpp>
#include <cli/templates.hpp>
#include <fstream>
#include <iostream>
#include <istream>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/stream.hpp>
#include <llove/tree.hpp>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <ranges>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif

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

static void print_help(
    const std::map<
        std::string,
        cli::OptionTemplate>& templates,
    const bool ascii)
{
    print_version();

    std::cerr << std::endl
              << "USAGE" << std::endl
              << " llove <PATTERN{=<FILTER>},...> <FILENAME>" << std::endl
              << std::endl
              << "FILENAME: empty, \"stdin\" or existing filename" << std::endl
              << std::endl
              << "OPTIONS" << std::endl;

    const auto console_width = get_console_width();
    cli::Table table(std::cerr, 3, console_width ? console_width : 120u, ascii);

    table << "PATTERN" << "FILTER" << "DESCRIPTION";

    for (auto& [template_pattern, template_type, template_filter, template_description] : templates | std::views::values)
    {
        std::string pattern_str;
        for (auto p = template_pattern.begin(); p != template_pattern.end(); ++p)
        {
            if (p != template_pattern.begin())
            {
                pattern_str += ", ";
            }
            pattern_str += *p;
        }
        table << pattern_str;

        std::string filter_str;
        if (template_type != cli::OptionTemplateType_Flag)
        {
            template_filter->Stringify(filter_str);
            if (template_type == cli::OptionTemplateType_Array)
            {
                filter_str += ",...";
            }
        }
        table << filter_str;

        table << template_description;
    }
}

int main(
    const int argc,
    const char* const* argv)
try
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    {
        if (argc == 1)
        {
            std::cerr << "no arguments. use '--help', '-h', '-?' or '?' for "
                         "help."
                      << std::endl;
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
            {
                return 0;
            }
        }

        auto input_filename = arguments.filename().empty() ? "stdin" : arguments.filename();

        llove::stream_ref<std::istream> input_stream_ref;
        if (input_filename == "stdin")
        {
            input_stream_ref = llove::stream_ref(&std::cin, false);
        }
        else
        {
            input_stream_ref = llove::stream_ref<std::ifstream>(input_filename);
        }

        if (input_stream_ref->fail())
        {
            std::cerr << "failed to open file '" << input_filename << "'" << std::endl;
            return 1;
        }

        llove::Machine machine{
            .Triple = llvm::sys::getDefaultTargetTriple(),
            .CPU = "generic",
            .Features = {},
            .Options = {},
            .Relocation = llvm::Reloc::PIC_,
        };

        (void) arguments.value("triple", machine.Triple);
        (void) arguments.value("cpu", machine.CPU);
        (void) arguments.array("features", machine.Features);
        (void) arguments.value("relocation", machine.Relocation);

        if (std::vector<int> version; arguments.array("option-binutils-version", version))
        {
            machine.Options.BinutilsVersion = { version[0], version[1] };
        }
        machine.Options.UnsafeFPMath = arguments.flag("option-unsafe-fp-math");
        machine.Options.NoInfsFPMath = arguments.flag("option-no-infs-fp-math");
        machine.Options.NoNaNsFPMath = arguments.flag("option-no-nans-fp-math");
        machine.Options.NoTrappingFPMath = arguments.flag("option-no-trapping-fp-math");
        machine.Options.NoSignedZerosFPMath = arguments.flag("option-no-signed-zeros-fp-math");
        machine.Options.ApproxFuncFPMath = arguments.flag("option-approx-func-fp-math");
        machine.Options.EnableAIXExtendedAltivecABI = arguments.flag("option-enable-aix-extended-altivec-abi");
        machine.Options.HonorSignDependentRoundingFPMathOption = arguments.flag("option-honor-sign-dependent-rounding-fp-math");
        machine.Options.NoZerosInBSS = arguments.flag("option-no-zeros-in-bss");
        machine.Options.GuaranteedTailCallOpt = arguments.flag("option-guaranteed-tail-call-opt");
        machine.Options.StackSymbolOrdering = arguments.flag("option-stack-symbol-ordering");
        machine.Options.EnableFastISel = arguments.flag("option-enable-fast-isel");
        machine.Options.EnableGlobalISel = arguments.flag("option-enable-global-isel");
        (void) arguments.value("option-global-isel-abort", machine.Options.GlobalISelAbort);
        (void) arguments.value(
            "option-swift-async-frame-pointer",
            machine.Options.SwiftAsyncFramePointer);
        machine.Options.UseInitArray = arguments.flag("option-use-init-array");
        machine.Options.DisableIntegratedAS = arguments.flag("option-disable-integrated-as");
        machine.Options.FunctionSections = arguments.flag("option-function-sections");
        machine.Options.DataSections = arguments.flag("option-data-sections");
        machine.Options.IgnoreXCOFFVisibility = arguments.flag("option-ignore-xcoff-visibility");
        machine.Options.XCOFFTracebackTable = arguments.flag("option-xcoff-traceback-table");
        machine.Options.UniqueSectionNames = arguments.flag("option-unique-section-names");
        machine.Options.UniqueBasicBlockSectionNames = arguments.flag("option-unique-basic-block-section-names");
        machine.Options.TrapUnreachable = arguments.flag("option-trap-unreachable");
        machine.Options.NoTrapAfterNoreturn = arguments.flag("option-no-trap-after-noreturn");
        machine.Options.TLSSize = arguments.flag("option-tls-size");
        machine.Options.EmulatedTLS = arguments.flag("option-emulated-tls");
        machine.Options.EnableTLSDESC = arguments.flag("option-enable-tls-desc");
        machine.Options.EnableIPRA = arguments.flag("option-enable-ipra");
        machine.Options.EmitStackSizeSection = arguments.flag("option-emit-stack-size-section");
        machine.Options.EnableMachineOutliner = arguments.flag("option-enable-machine-outliner");
        machine.Options.EnableMachineFunctionSplitter = arguments.flag("option-enable-machine-function-splitter");
        machine.Options.SupportsDefaultOutlining = arguments.flag("option-supports-default-outlining");
        machine.Options.EmitAddrsig = arguments.flag("option-emit-addrsig");
        (void) arguments.value("option-bb-sections", machine.Options.BBSections);
        machine.Options.EmitCallSiteInfo = arguments.flag("option-emit-call-site-info");
        machine.Options.SupportsDebugEntryValues = arguments.flag("option-supports-debug-entry-values");
        machine.Options.EnableDebugEntryValues = arguments.flag("option-enable-debug-entry-values");
        machine.Options.ValueTrackingVariableLocations = arguments.flag("option-value-tracking-variable-locations");
        machine.Options.ForceDwarfFrameSection = arguments.flag("option-force-dwarf-frame-section");
        machine.Options.XRayFunctionIndex = arguments.flag("option-xray-function-index");
        machine.Options.DebugStrictDwarf = arguments.flag("option-debug-strict-dwarf");
        machine.Options.Hotpatch = arguments.flag("option-hotpatch");
        machine.Options.PPCGenScalarMASSEntries = arguments.flag("option-ppc-gen-scalar-mass-entries");
        machine.Options.JMCInstrument = arguments.flag("option-jmc-instrument");
        machine.Options.EnableCFIFixup = arguments.flag("option-enable-cfi-fixup");
        machine.Options.MisExpect = arguments.flag("option-mis-expect");
        machine.Options.XCOFFReadOnlyPointers = arguments.flag("option-xcoff-read-only-pointers");
        (void) arguments.value("option-stack-usage-output", machine.Options.StackUsageOutput);
        machine.Options.LoopAlignment = arguments.flag("option-loop-alignment");
        (void) arguments.value("option-float-abi-type", machine.Options.FloatABIType);
        (void) arguments.value("option-allow-fp-op-fusion", machine.Options.AllowFPOpFusion);
        (void) arguments.value("option-thread-model", machine.Options.ThreadModel);
        (void) arguments.value("option-eabi-version", machine.Options.EABIVersion);
        (void) arguments.value("option-debugger-tuning", machine.Options.DebuggerTuning);
        if (std::vector<llvm::DenormalMode::DenormalModeKind> values; arguments.array("option-fp-denormal-mode", values))
        {
            machine.Options.setFPDenormalMode({ values[0], values[1] });
        }
        if (std::vector<llvm::DenormalMode::DenormalModeKind> values; arguments.array("option-fp32-denormal-mode", values))
        {
            machine.Options.setFP32DenormalMode({ values[0], values[1] });
        }
        (void) arguments.value("option-exception-model", machine.Options.ExceptionModel);

        machine.Options.MCOptions.MCRelaxAll = arguments.flag("mc-option-relax-all");
        machine.Options.MCOptions.MCNoExecStack = arguments.flag("mc-option-no-exec-stack");
        machine.Options.MCOptions.MCFatalWarnings = arguments.flag("mc-option-fatal-warnings");
        machine.Options.MCOptions.MCNoWarn = arguments.flag("mc-option-no-warn");
        machine.Options.MCOptions.MCNoDeprecatedWarn = arguments.flag("mc-option-no-deprecated-warn");
        machine.Options.MCOptions.MCNoTypeCheck = arguments.flag("mc-option-no-type-check");
        machine.Options.MCOptions.MCSaveTempLabels = arguments.flag("mc-option-save-temp-labels");
        machine.Options.MCOptions.MCIncrementalLinkerCompatible = arguments.flag("mc-option-incremental-linker-compatible");
        machine.Options.MCOptions.ShowMCEncoding = arguments.flag("mc-option-show-mc-encoding");
        machine.Options.MCOptions.ShowMCInst = arguments.flag("mc-option-show-mc-inst");
        machine.Options.MCOptions.AsmVerbose = arguments.flag("mc-option-asm-verbose");
        machine.Options.MCOptions.PreserveAsmComments = arguments.flag("mc-option-preserve-asm-comments");
        machine.Options.MCOptions.Dwarf64 = arguments.flag("mc-option-dwarf64");
        (void) arguments.value(
            "mc-option-emit-dwarf-unwind",
            machine.Options.MCOptions.EmitDwarfUnwind);
        (void) arguments.value(
            "mc-option-dwarf-version",
            machine.Options.MCOptions.DwarfVersion);
        (void) arguments.value(
            "mc-option-use-dwarf-directory",
            machine.Options.MCOptions.MCUseDwarfDirectory);
        (void) arguments.value("mc-option-abi-name", machine.Options.MCOptions.ABIName);
        (void) arguments.value(
            "mc-option-assembly-language",
            machine.Options.MCOptions.AssemblyLanguage);
        (void) arguments.value(
            "mc-option-split-dwarf-file",
            machine.Options.MCOptions.SplitDwarfFile);
        (void) arguments.value(
            "mc-option-as-secure-log-file",
            machine.Options.MCOptions.AsSecureLogFile);
        machine.Options.MCOptions.EmitCompactUnwindNonCanonical = arguments.flag("mc-option-emit-compact-unwind-non-canonical");
        machine.Options.MCOptions.PPCUseFullRegisterNames = arguments.flag("mc-option-ppc-use-full-register-names");

        auto debug = arguments.flag("debug");
        auto optimized = arguments.has_value_and_is_not("level", "0");
        auto profiling = arguments.flag("profiling");

        std::set<std::filesystem::path> includes;
        (void) arguments.set("include", includes);

        auto emission = llvm::DICompileUnit::DebugEmissionKind::FullDebug;
        (void) arguments.value("debug-kind", emission);

        llove::Context context;
        llove::Builder builder(
            context,
            machine,
            debug,
            optimized,
            profiling,
            emission,
            input_filename,
            arguments.BuildCommandLine());
        llove::Parser parser(context, *input_stream_ref, input_filename, includes);

        std::string print_filename;
        auto has_print_filename = arguments.value("print-output", print_filename);

        llove::stream_ref<std::ostream> print_stream_ref;
        if (!has_print_filename || print_filename == "stderr")
        {
            print_stream_ref = llove::stream_ref(&std::cerr, false);
        }
        else if (print_filename == "stdout")
        {
            print_stream_ref = llove::stream_ref(&std::cout, false);
        }
        else
        {
            print_stream_ref = llove::stream_ref<std::ofstream>(print_filename);
        }
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
            if (auto ptr = parser.Parse())
            {
                if (print_llove)
                    *print_stream_ref << ptr << std::endl;

                ptr->Gen(builder);
            }
        }

        context.InstantiateReflections(builder);

        std::string output_filename;
        auto has_output_filename = arguments.value("output", output_filename);

        llove::stream_ref<std::ostream> output_stream_ref;
        if (!has_output_filename || output_filename == "stdout")
        {
            output_stream_ref = llove::stream_ref(&std::cout, false);
        }
        else if (output_filename == "stderr")
        {
            output_stream_ref = llove::stream_ref(&std::cerr, false);
        }
        else
        {
            output_stream_ref = llove::stream_ref<std::ofstream>(output_filename, std::ios_base::out | std::ios_base::binary);
        }
        if (output_stream_ref->fail())
        {
            std::cerr << "failed to open file '" << output_filename << "'" << std::endl;
            return 1;
        }

        auto code_gen_type = llvm::CodeGenFileType::ObjectFile;
        (void) arguments.value("format", code_gen_type);

        auto optimization_level = llvm::OptimizationLevel::O0;
        (void) arguments.value("level", optimization_level);

        builder.Seal(print_llvm, *print_stream_ref, *output_stream_ref, code_gen_type, optimization_level);
    }

    return 0;
}
catch (const llove::ref_exception<llove::ErrorStack>& cause)
{
    cause->Print(std::cerr);
    return 1;
}
