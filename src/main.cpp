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
#include <llove/tree.hpp>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/TargetParser/Host.h>

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
// --option-ppc-gen-scalar-mass-entities
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

static void print_help(const std::map<std::string, cli::OptionTemplate> &templates)
{
    print_version();

    std::cerr
            << std::endl
            << "USAGE" << std::endl
            << " llove <PATTERN{=<FILTER>},...> <filename>" << std::endl
            << std::endl
            << "OPTIONS" << std::endl;

    cli::Table table(std::cerr, 3);

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

int main(const int argc, const char *const *argv)
{
    static const std::map<std::string_view, llvm::CodeGenFileType> CODE_GEN_FILE_TYPE
    {
        { "asm", llvm::CodeGenFileType::AssemblyFile },
        { "obj", llvm::CodeGenFileType::ObjectFile },
    };

    static const std::map<std::string_view, llvm::Reloc::Model> RELOC_MODEL
    {
        { "static", llvm::Reloc::Static },
        { "pic", llvm::Reloc::PIC_ },
        { "dynamic-no-pic", llvm::Reloc::DynamicNoPIC },
        { "ropi", llvm::Reloc::ROPI },
        { "rwpi", llvm::Reloc::RWPI },
        { "ropi-rwpi", llvm::Reloc::ROPI_RWPI },
    };

    static const std::map<std::string_view, llvm::OptimizationLevel> OPTIMIZATION_LEVEL
    {
        { "0", llvm::OptimizationLevel::O0 },
        { "1", llvm::OptimizationLevel::O1 },
        { "2", llvm::OptimizationLevel::O2 },
        { "3", llvm::OptimizationLevel::O3 },
        { "s", llvm::OptimizationLevel::Os },
        { "z", llvm::OptimizationLevel::Oz },
    };

    if (argc == 1)
    {
        std::cerr << "no arguments. use '--help', '-h', '-?' or '?' for help." << std::endl;
        return 1;
    }

    cli::Arguments arguments(argv + 1, argv + argc);

    if (arguments.flag("help"))
    {
        print_help(arguments.templates());
        return 0;
    }

    if (arguments.flag("version"))
    {
        print_version();
        if (arguments.filename().empty())
            return 0;
    }

    if (arguments.filename().empty())
    {
        std::cerr << "missing filename. use '--help', '-h', '-?' or '?' for help." << std::endl;
        return 1;
    }

    std::ifstream stream(arguments.filename());
    if (!stream.is_open())
        return 1;

    llove::Context types;
    llove::Builder builder(types, arguments.filename());
    llove::Parser parser(types, builder, stream, arguments.filename());

    llove::SealInfo seal_info
    {
        .Print = false,
        .PrintStream = &std::cerr,
        .OutputStream = &std::cout,
        .Format = llvm::CodeGenFileType::ObjectFile,
        .Triple = llvm::sys::getDefaultTargetTriple(),
        .CPU = "generic",
        .Features = {},
        .Options = {},
        .Relocation = llvm::Reloc::PIC_,
        .Level = llvm::OptimizationLevel::O2,
    };

    std::string print_filename, output_filename;
    auto print_file = arguments.value("print-output", print_filename)
                      && print_filename != "stdout"
                      && print_filename != "stderr";
    auto output_file = arguments.value("output", output_filename)
                       && output_filename != "stdout"
                       && output_filename != "stderr";

    if (print_file)
        seal_info.PrintStream = new std::ofstream(print_filename);
    if (output_file)
        seal_info.OutputStream = new std::ofstream(output_filename);

    auto print_llove = false;

    if (std::vector<std::string> print; arguments.array("print", print))
    {
        std::set print_set(print.begin(), print.end());
        print_llove = print_set.contains("llove");
        seal_info.Print = print_set.contains("llvm");
    }

    while (parser.Ok())
        try
        {
            if (auto ptr = parser.Parse())
            {
                if (print_llove)
                    *seal_info.PrintStream << ptr << std::endl;
                ptr->Gen(builder);
            }
        }
        catch (const llove::ErrorStack *cause)
        {
            cause->Print(std::cerr);
            return 1;
        }

    if (std::string format; arguments.value("format", format))
        seal_info.Format = CODE_GEN_FILE_TYPE.at(format);

    (void) arguments.value("triple", seal_info.Triple);
    (void) arguments.value("cpu", seal_info.CPU);
    (void) arguments.array("features", seal_info.Features);

    if (std::string relocation; arguments.value("relocation", relocation))
        seal_info.Relocation = RELOC_MODEL.at(relocation);

    if (std::string level; arguments.value("level", level))
        seal_info.Level = OPTIMIZATION_LEVEL.at(level);

    builder.Seal(seal_info);

    if (print_file)
        delete seal_info.PrintStream;
    if (output_file)
        delete seal_info.OutputStream;

    return 0;
}
