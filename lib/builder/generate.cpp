#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

std::string llove::Builder::Mangle(const Function &function)
{
    if (function.IsInterface)
        return function.Name;

    auto mangled = '?' + std::to_string(function.Name.size()) + '_' + function.Name;

    if (function.Class)
    {
        auto &class_name = function.Class->GetName();
        mangled += (function.IsMutable ? 'm' : 'c') + std::to_string(class_name.size()) + '_' + class_name;
    }

    if (function.Variadic.first)
        mangled += 'v';

    mangled += std::to_string(function.Parameters.size()) + '_';
    for (auto &parameter : function.Parameters)
        mangled += parameter.Info.Mangle();

    return mangled + function.Result.Mangle();
}

llove::FunctionReference llove::Builder::GenFunction(const Function &function, const bool register_function)
{
    const auto mangled = Mangle(function);

    std::vector<Field> type_parameters;
    for (auto &parameter : function.Parameters)
        type_parameters.emplace_back(parameter.Info);

    std::optional<Field> self;
    FunctionType::Ptr callee_type;

    if (function.Class)
    {
        self = Field(function.IsMutable, true, function.Class);
        callee_type = m_Context.GetFunction(
            std::move(type_parameters),
            function.Variadic.first,
            function.Result,
            *self);
    }
    else
    {
        callee_type = m_Context.GetFunction(std::move(type_parameters), function.Variadic.first, function.Result);
    }

    const auto callee = GetOrCreateFunction(mangled, callee_type, function.IsExport || function.IsInterface);

    auto reference = register_function
                         ? PushFunction(function.IsExposed, function.IsImplicit, function.Name, callee_type, callee)
                         : FunctionReference{
                             .IsExposed = function.IsExposed,
                             .IsImplicit = function.IsImplicit,
                             .Name = function.Name,
                             .Type = callee_type,
                             .Callee = callee,
                         };

    if (!function.Content)
        return reference;

    Assert(callee->empty(), "function is already defined");

    m_DebugBuilder.BeginFunction(*this, function.Name, function.Loc, callee_type, mangled, callee);

    m_Parent = callee;
    m_Class = function.Class;
    m_Result = function.Result;

    const auto entry_block = CreateBlock("entry", callee);
    m_LLVMBuilder.SetInsertPoint(entry_block);

    m_DebugBuilder.EmitLoc(*this);
    PushFrame();
    GenParameters(callee, function.Parameters, function.Variadic, self);
    function.Content->Gen(*this);
    PopFrame();

    m_DebugBuilder.EndFunction();

    for (auto &block : *callee)
    {
        if (block.getTerminator())
            continue;

        if (function.Result.GetType()->IsVoid())
        {
            m_LLVMBuilder.SetInsertPoint(&block);
            m_LLVMBuilder.CreateRetVoid();
            continue;
        }

        callee->print(llvm::errs());
        Error("not all paths yield");
    }

    m_LLVMBuilder.ClearInsertionPoint();

    if (!verifyFunction(*callee, &llvm::errs()))
        return reference;

    callee->print(llvm::errs());
    Error("function has errors");
}

void llove::Builder::GenParameters(
    llvm::Function *parent,
    const std::vector<Parameter> &parameters,
    const std::pair<bool, std::string> &variadic,
    const std::optional<Field> &self)
{
    auto iterator = parent->arg_begin();
    auto index = 1u;

    if (self)
    {
        const auto argument = iterator++;
        argument->setName("self");

        auto storage = Value::CreateL(self->GetType(), argument, self->IsMutable());

        m_DebugBuilder.CreateParameter(*this, "self", index++, storage);
        SetValue("self", std::move(storage));
    }

    for (auto &parameter : parameters)
    {
        const auto argument = iterator++;
        argument->setName(parameter.Name);

        ValuePtr storage;
        if (parameter.Info.IsReference())
        {
            storage = Value::CreateL(parameter.Info.GetType(), argument, parameter.Info.IsMutable());
        }
        else if (parameter.Info.GetType()->IsClass())
        {
            const auto pointer = CreateAlloca(parameter.Info.GetType()->GenIR(*this), parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(parameter.Info.GetType(), pointer, parameter.Info.IsMutable());

            PushDestructor(pointer, As<ClassType>(parameter.Info.GetType()));
        }
        else if (parameter.Info.IsMutable())
        {
            const auto pointer = CreateAlloca(parameter.Info.GetType()->GenIR(*this), parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(parameter.Info.GetType(), pointer, parameter.Info.IsMutable());
        }
        else
        {
            storage = Value::CreateR(parameter.Info.GetType(), argument);
        }

        m_DebugBuilder.CreateParameter(*this, parameter.Name, index++, storage);
        SetValue(parameter.Name, std::move(storage));
    }

    if (variadic.first && !variadic.second.empty())
    {
        const auto argument = iterator;
        argument->setName(variadic.second);

        const auto pointer = CreateAlloca(GetVariadicType(), parent);
        CreateStore(argument, pointer);

        auto storage = Value::CreateL(m_Context.GetVariadic(), pointer, true);

        m_DebugBuilder.CreateParameter(*this, variadic.second, index, storage);
        SetValue(variadic.second, std::move(storage));
    }
}
