#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <ranges>
#include <string>

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
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>

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

    for (auto &[template_pattern, template_type, template_filter, template_description] :
         templates | std::views::values)
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
    const char *const*argv) try
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

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

        (void) arguments.value("option-allow-fp-op-fusion", machine.Options.AllowFPOpFusion);
        machine.Options.BBAddrMap = arguments.flag("option-bb-addr-map");
        (void) arguments.value("option-bb-sections", machine.Options.BBSections);
        if (std::vector<int> version; arguments.array("option-binutils-version", version))
            machine.Options.BinutilsVersion = { version[0], version[1] };
        machine.Options.DataSections = arguments.flag("option-data-sections");
        (void) arguments.value("option-debugger-tuning", machine.Options.DebuggerTuning);
        machine.Options.DebugStrictDwarf = arguments.flag("option-debug-strict-dwarf");
        machine.Options.DisableIntegratedAS = arguments.flag("option-disable-integrated-as");
        (void) arguments.value("option-eabi-version", machine.Options.EABIVersion);
        machine.Options.EmitAddrsig = arguments.flag("option-emit-addrsig");
        machine.Options.EmitCallGraphSection = arguments.flag("option-emit-call-graph-section");
        machine.Options.EmitCallSiteInfo = arguments.flag("option-emit-call-site-info");
        machine.Options.EmitStackSizeSection = arguments.flag("option-emit-stack-size-section");
        machine.Options.EmulatedTLS = arguments.flag("option-emulated-tls");
        machine.Options.EnableAIXExtendedAltivecABI = arguments.flag("option-enable-aix-extended-altivec-abi");
        machine.Options.EnableCFIFixup = arguments.flag("option-enable-cfi-fixup");
        machine.Options.EnableDebugEntryValues = arguments.flag("option-enable-debug-entry-values");
        machine.Options.EnableFastISel = arguments.flag("option-enable-fast-isel");
        machine.Options.EnableGlobalISel = arguments.flag("option-enable-global-isel");
        machine.Options.EnableIPRA = arguments.flag("option-enable-ipra");
        machine.Options.EnableMachineFunctionSplitter = arguments.flag("option-enable-machine-function-splitter");
        machine.Options.EnableMachineOutliner = arguments.flag("option-enable-machine-outliner");
        machine.Options.EnableStaticDataPartitioning = arguments.flag("option-enable-static-data-partitioning");
        machine.Options.EnableTLSDESC = arguments.flag("option-enable-tls-desc");
        (void) arguments.value("option-exception-model", machine.Options.ExceptionModel);
        (void) arguments.value("option-float-abi-type", machine.Options.FloatABIType);
        machine.Options.ForceDwarfFrameSection = arguments.flag("option-force-dwarf-frame-section");
        machine.Options.FunctionSections = arguments.flag("option-function-sections");
        (void) arguments.value("option-global-isel-abort", machine.Options.GlobalISelAbort);
        machine.Options.GuaranteedTailCallOpt = arguments.flag("option-guaranteed-tail-call-opt");
        machine.Options.HonorSignDependentRoundingFPMathOption = arguments.flag(
            "option-honor-sign-dependent-rounding-fp-math");
        machine.Options.Hotpatch = arguments.flag("option-hotpatch");
        machine.Options.IgnoreXCOFFVisibility = arguments.flag("option-ignore-xcoff-visibility");
        machine.Options.JMCInstrument = arguments.flag("option-jmc-instrument");
        machine.Options.LoopAlignment = arguments.flag("option-loop-alignment");

        (void) arguments.value("mc-option-abi-name", machine.Options.MCOptions.ABIName);
        machine.Options.MCOptions.Argv0 = argv[0];
        machine.Options.MCOptions.AsmVerbose = arguments.flag("mc-option-asm-verbose");
        (void) arguments.value("mc-option-as-secure-log-file", machine.Options.MCOptions.AsSecureLogFile);
        (void) arguments.value("mc-option-assembly-language", machine.Options.MCOptions.AssemblyLanguage);
        machine.Options.MCOptions.CommandlineArgs = arguments.BuildCommandLine();
        (void) arguments.value("mc-option-compress-debug-sections", machine.Options.MCOptions.CompressDebugSections);
        machine.Options.MCOptions.Crel = arguments.flag("mc-option-crel");
        machine.Options.MCOptions.Dwarf64 = arguments.flag("mc-option-dwarf64");
        (void) arguments.value("mc-option-dwarf-version", machine.Options.MCOptions.DwarfVersion);
        machine.Options.MCOptions.EmitCompactUnwindNonCanonical = arguments.flag(
            "mc-option-emit-compact-unwind-non-canonical");
        (void) arguments.value("mc-option-emit-dwarf-unwind", machine.Options.MCOptions.EmitDwarfUnwind);
        machine.Options.MCOptions.EmitSFrameUnwind = arguments.flag("mc-option-emit-sframe-unwind");
        machine.Options.MCOptions.FDPIC = arguments.flag("mc-option-fdpic");
        (void) arguments.array("mc-option-ias-search-paths", machine.Options.MCOptions.IASSearchPaths);
        machine.Options.MCOptions.ImplicitMapSyms = arguments.flag("mc-option-implicit-map-syms");
        (void) arguments.array("mc-option-inst-printer-options", machine.Options.MCOptions.InstPrinterOptions);
        machine.Options.MCOptions.MCFatalWarnings = arguments.flag("mc-option-fatal-warnings");
        machine.Options.MCOptions.MCIncrementalLinkerCompatible = arguments.flag(
            "mc-option-incremental-linker-compatible");
        machine.Options.MCOptions.MCNoDeprecatedWarn = arguments.flag("mc-option-no-deprecated-warn");
        machine.Options.MCOptions.MCNoExecStack = arguments.flag("mc-option-no-exec-stack");
        machine.Options.MCOptions.MCNoTypeCheck = arguments.flag("mc-option-no-type-check");
        machine.Options.MCOptions.MCNoWarn = arguments.flag("mc-option-no-warn");
        machine.Options.MCOptions.MCRelaxAll = arguments.flag("mc-option-relax-all");
        machine.Options.MCOptions.MCSaveTempLabels = arguments.flag("mc-option-save-temp-labels");
        (void) arguments.value("mc-option-use-dwarf-directory", machine.Options.MCOptions.MCUseDwarfDirectory);
        if (unsigned value; arguments.value("mc-option-output-asm-variant", value))
            machine.Options.MCOptions.OutputAsmVariant = value;
        machine.Options.MCOptions.PPCUseFullRegisterNames = arguments.flag("mc-option-ppc-use-full-register-names");
        machine.Options.MCOptions.PreserveAsmComments = arguments.flag("mc-option-preserve-asm-comments");
        machine.Options.MCOptions.ShowMCEncoding = arguments.flag("mc-option-show-mc-encoding");
        machine.Options.MCOptions.ShowMCInst = arguments.flag("mc-option-show-mc-inst");
        (void) arguments.value("mc-option-split-dwarf-file", machine.Options.MCOptions.SplitDwarfFile);
        machine.Options.MCOptions.X86RelaxRelocations = arguments.flag("mc-option-x86-relax-relocations");
        machine.Options.MCOptions.X86Sse2Avx = arguments.flag("mc-option-x86-sse2avx");

        machine.Options.MisExpect = arguments.flag("option-mis-expect");
        machine.Options.NoInfsFPMath = arguments.flag("option-no-infs-fp-math");
        machine.Options.NoNaNsFPMath = arguments.flag("option-no-nans-fp-math");
        machine.Options.NoSignedZerosFPMath = arguments.flag("option-no-signed-zeros-fp-math");
        machine.Options.NoTrapAfterNoreturn = arguments.flag("option-no-trap-after-noreturn");
        machine.Options.NoTrappingFPMath = arguments.flag("option-no-trapping-fp-math");
        machine.Options.NoZerosInBSS = arguments.flag("option-no-zeros-in-bss");
        (void) arguments.value("option-object-filename-for-debug", machine.Options.ObjectFilenameForDebug);
        machine.Options.PPCGenScalarMASSEntries = arguments.flag("option-ppc-gen-scalar-mass-entries");
        machine.Options.SeparateNamedSections = arguments.flag("option-separate-named-sections");
        machine.Options.StackSymbolOrdering = arguments.flag("option-stack-symbol-ordering");
        (void) arguments.value("option-stack-usage-output", machine.Options.StackUsageOutput);
        machine.Options.SupportsDebugEntryValues = arguments.flag("option-supports-debug-entry-values");
        machine.Options.SupportsDefaultOutlining = arguments.flag("option-supports-default-outlining");
        (void) arguments.value("option-swift-async-frame-pointer", machine.Options.SwiftAsyncFramePointer);
        (void) arguments.value("option-thread-model", machine.Options.ThreadModel);
        machine.Options.TLSSize = arguments.flag("option-tls-size");
        machine.Options.TrapUnreachable = arguments.flag("option-trap-unreachable");
        machine.Options.UniqueBasicBlockSectionNames = arguments.flag("option-unique-basic-block-section-names");
        machine.Options.UniqueSectionNames = arguments.flag("option-unique-section-names");
        machine.Options.UseInitArray = arguments.flag("option-use-init-array");
        machine.Options.ValueTrackingVariableLocations = arguments.flag("option-value-tracking-variable-locations");
        (void) arguments.value("option-vec-lib", machine.Options.VecLib);
        machine.Options.VerifyArgABICompliance = arguments.flag("option-verify-arg-abi-compliance");
        machine.Options.XCOFFReadOnlyPointers = arguments.flag("option-xcoff-read-only-pointers");
        machine.Options.XCOFFTracebackTable = arguments.flag("option-xcoff-traceback-table");
        machine.Options.XRayFunctionIndex = arguments.flag("option-xray-function-index");

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
            if (auto ptr = parser.Parse())
            {
                if (print_llove)
                    *print_stream_ref << ptr << std::endl;

                ptr->Gen(builder);
            }

        std::string output_filename;
        auto has_output_filename = arguments.value("output", output_filename);

        llove::stream_ref<std::ostream> output_stream_ref;
        if (!has_output_filename || output_filename == "stdout")
            output_stream_ref = llove::stream_ref(&std::cout, false);
        else if (output_filename == "stderr")
            output_stream_ref = llove::stream_ref(&std::cerr, false);
        else
            output_stream_ref =
                    llove::stream_ref<std::ofstream>(output_filename, std::ios_base::out | std::ios_base::binary);

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
catch (const llove::ref_exception<llove::ErrorStack> &cause)
{
    cause->Print(std::cerr);
    return 1;
}
