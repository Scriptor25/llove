#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

llove::Builder::Builder(Context &types, const std::filesystem::path &filepath)
    : m_Types(types),
      m_Builder(m_Context),
      m_Module("main", m_Context),
      m_DIBuilder(m_Module)
{
    m_Module.setSourceFileName(filepath.string());

    m_CompileUnit = m_DIBuilder.createCompileUnit(
        llvm::dwarf::DW_LANG_C,
        m_DIBuilder.createFile(filepath.string(), filepath.parent_path().string()),
        "LLove",
        false,
        "",
        0u);
}

llove::Context &llove::Builder::GetTypes() const
{
    return m_Types;
}

llvm::DIScope *llove::Builder::GetDbgScope() const
{
    return m_Stack.empty() ? m_CompileUnit : m_Stack.back().Scope;
}

llvm::DIFile *llove::Builder::GetDbgFile() const
{
    return GetDbgScope()->getFile();
}

std::string llove::Builder::Mangle(
    const bool interface,
    const ClassType::Ptr &class_type,
    const bool mutable_,
    const std::string &name,
    const std::vector<Parameter> &parameters,
    const bool vararg,
    const Field &result)
{
    if (interface)
        return name;

    auto mangled = '?' + std::to_string(name.size()) + '_' + name;

    if (class_type)
    {
        auto &class_name = class_type->GetName();
        mangled += (mutable_ ? 'm' : 'c') + std::to_string(class_name.size()) + '_' + class_name;
    }

    if (vararg)
        mangled += 'v';

    mangled += std::to_string(parameters.size()) + '_';
    for (auto &[info_, name_] : parameters)
        mangled += info_.Mangle();

    return mangled + result.Mangle();
}

llvm::Value *llove::Builder::CreateGlobalString(const std::string &value)
{
    return m_Builder.CreateGlobalStringPtr(value, {}, 0, &m_Module);
}

llove::FunctionReference &llove::Builder::GenFunction(const FunctionInfo &fn)
{
    const auto mangled = Mangle(
        fn.Interface,
        fn.Class,
        fn.Mutable,
        fn.Name,
        fn.Parameters,
        fn.VarArg,
        fn.Result);

    std::vector<Field> type_parameters;
    for (auto &[info, name] : fn.Parameters)
        type_parameters.emplace_back(info);

    Field self;
    FunctionType::Ptr function_type;

    if (fn.Class)
    {
        self = {
            .Mutable = fn.Mutable,
            .Reference = true,
            .Type = fn.Class,
        };
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result, self);
    }
    else
    {
        function_type = m_Types.GetFunction(type_parameters, fn.VarArg, fn.Result);
    }

    const auto function = GetOrCreateFunction(mangled, function_type, fn.Interface);
    auto &reference = PushFunction(fn.Expose, fn.Implicit, fn.Name, function_type, function);

    if (!fn.Content)
        return reference;

    Assert(function->empty(), "function is already defined");

    const auto dbg_unit = m_DIBuilder.createFile(m_CompileUnit->getFilename(), m_CompileUnit->getDirectory());
    const auto dbg_subprogram = m_DIBuilder.createFunction(
        dbg_unit,
        fn.Name,
        mangled,
        dbg_unit,
        fn.Loc.Row,
        function_type->GenDbgFunction(*this),
        fn.Loc.Row,
        llvm::DINode::FlagPrototyped,
        llvm::DISubprogram::SPFlagDefinition);
    function->setSubprogram(dbg_subprogram);

    m_Parent = function;
    m_Class = fn.Class;
    m_Result = fn.Result;

    const auto entry_block = CreateBlock("entry", function);
    m_Builder.SetInsertPoint(entry_block);

    PushFrame(dbg_subprogram);

    EmitLoc();
    GenParameters(function, fn.Parameters, self);
    fn.Content->Gen(*this);
    PopFrame();

    for (auto &block : *function)
    {
        if (block.getTerminator())
            continue;
        if (fn.Result.Type->IsVoid())
        {
            m_Builder.SetInsertPoint(&block);
            m_Builder.CreateRetVoid();
            continue;
        }
        Error("not all paths yield");
    }

    Assert(!verifyFunction(*function, &llvm::errs()), "function has errors");

    return reference;
}

void llove::Builder::GenParameters(
    llvm::Function *function,
    const std::vector<Parameter> &parameters,
    const Field &self)
{
    auto offset = 0u;
    if (self)
    {
        offset = 1u;

        const auto argument = function->getArg(0);
        argument->setName("self");

        const auto dbg_var = m_DIBuilder.createParameterVariable(
            GetDbgScope(),
            "self",
            0u,
            GetDbgFile(),
            0u,
            self.Type->GenDbg(*this),
            true);

        m_DIBuilder.insertDeclare(
            argument,
            dbg_var,
            m_DIBuilder.createExpression(),
            llvm::DILocation::get(m_Context, 0u, 0u, GetDbgScope()),
            m_Builder.GetInsertBlock());

        SetValue("self", Value::CreateL(self.Type, argument, self.Mutable));
    }

    for (unsigned i = 0; i < function->arg_size() - offset; ++i)
    {
        auto &[info, name] = parameters.at(i);

        const auto argument = function->getArg(i + offset);
        argument->setName(name);

        ValuePtr storage;
        if (info.Reference)
        {
            storage = Value::CreateL(info.Type, argument, info.Mutable);
        }
        else if (info.Type->IsClass())
        {
            const auto pointer = CreateAlloca(info.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(info.Type, pointer, info.Mutable);

            auto class_type = As<ClassType>(info.Type);
            if (const auto destructor = class_type->GetDestructor())
            {
                const auto &reference = GenFunction(
                    {
                        .Class = std::move(class_type),
                        .Mutable = destructor->Mutable,
                        .Expose = destructor->Expose,
                        .Name = destructor->Name,
                        .Result = destructor->Result,
                    });

                PushDestructor(
                    pointer,
                    {
                        reference.Type->GenFunction(*this),
                        reference.Callee,
                    });
            }
        }
        else if (info.Mutable)
        {
            const auto pointer = CreateAlloca(info.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(info.Type, pointer, info.Mutable);
        }
        else
        {
            storage = Value::CreateR(info.Type, argument);
        }

        CreateDbgParameter(name, i, storage);
        SetValue(name, storage);
    }
}
