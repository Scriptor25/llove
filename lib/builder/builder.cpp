#include <ranges>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

llove::Builder::Builder(Context &types)
    : m_Types(types),
      m_Builder(m_Context),
      m_Module("main", m_Context),
      m_Parent(nullptr)
{
}

llove::Context &llove::Builder::GetTypes() const
{
    return m_Types;
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
    auto &reference = PushFunction(fn.Expose, fn.Name, function_type, function);

    if (!fn.Content)
        return reference;

    m_Parent = function;
    m_Class = fn.Class;
    m_Result = fn.Result;

    const auto entry_block = CreateBlock("entry", function);
    m_Builder.SetInsertPoint(entry_block);

    PushFrame();
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

    const auto error = verifyFunction(*function, &llvm::errs());
    Assert(!error, "function has errors");

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

        SetValue("self", Value::CreateL(self.Type, argument, self.Mutable));
    }

    for (unsigned i = 0; i < function->arg_size() - offset; ++i)
    {
        auto &[info_, name_] = parameters.at(i);

        const auto argument = function->getArg(i + offset);
        argument->setName(name_);

        ValuePtr storage;
        if (info_.Reference)
        {
            storage = Value::CreateL(info_.Type, argument, info_.Mutable);
        }
        else if (info_.Type->IsClass())
        {
            const auto pointer = CreateAlloca(info_.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(info_.Type, pointer, info_.Mutable);

            auto class_type = As<ClassType>(info_.Type);
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
        else if (info_.Mutable)
        {
            const auto pointer = CreateAlloca(info_.Type, function);
            m_Builder.CreateStore(argument, pointer);

            storage = Value::CreateL(info_.Type, pointer, info_.Mutable);
        }
        else
        {
            storage = Value::CreateR(info_.Type, argument);
        }

        SetValue(name_, storage);
    }
}
