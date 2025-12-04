#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <llvm/IR/Verifier.h>

std::string llove::Builder::Mangle(const Function& function)
{
    if (function.IsInterface)
    {
        return function.Name;
    }

    auto mangled = '?' + std::to_string(function.Name.size()) + '_' + function.Name;

    if (function.Class)
    {
        auto& class_name = function.Class->GetName();
        mangled += (function.IsMutable ? 'm' : 'c') + std::to_string(class_name.size()) + '_' + class_name;
    }

    if (function.Variadic.first)
    {
        mangled += 'v';
    }

    mangled += std::to_string(function.Parameters.size()) + '_';
    for (auto& parameter : function.Parameters)
    {
        mangled += parameter.Info.Mangle();
    }

    return mangled + function.Result.Mangle();
}

llove::FunctionReference llove::Builder::GenFunction(
    const Function& function,
    const bool register_function)
{
    const auto mangled = Mangle(function);

    std::vector<Field> type_parameters;
    for (auto& parameter : function.Parameters)
    {
        type_parameters.emplace_back(parameter.Info);
    }

    std::optional<Field> self;
    FunctionType::Ptr callee_type;

    if (function.Class)
    {
        self = Field(function.IsMutable, true, function.Class);
        callee_type = m_Context.GetFunction(
            function.Result,
            std::move(type_parameters),
            function.Variadic.first,
            *self);
    }
    else
    {
        callee_type = m_Context.GetFunction(
            function.Result,
            std::move(type_parameters),
            function.Variadic.first);
    }

    const auto callee = GetOrCreateFunction(
        mangled,
        callee_type,
        function.IsExport || function.IsInterface);

    auto reference = register_function ? PushFunction(
                                             function.IsPublic,
                                             function.IsImplicit,
                                             function.Name,
                                             callee_type,
                                             callee)
                                       : FunctionReference{
                                             .IsPublic = function.IsPublic,
                                             .IsImplicit = function.IsImplicit,
                                             .Name = function.Name,
                                             .Type = callee_type,
                                             .Callee = callee,
                                         };

    if (!function.Content)
    {
        return reference;
    }

    Assert(callee->empty(), "function is already defined");

    m_DebugBuilder.BeginFunction(*this, function.Name, function.Loc, callee_type, mangled, callee);

    m_Parent = callee;
    m_Class = function.Class;
    m_Result = function.Result;

    const auto entry_block = CreateBlock("entry", callee);
    m_LLVMBuilder.SetInsertPoint(entry_block);

    m_DebugBuilder.EmitLoc(*this);
    PushFrame();

    if (auto self_pointer = GenParameters(callee, function.Parameters, function.Variadic, self); self_pointer && function.Name == "create")
    {
        EmitLoc(function.Loc);

        auto self_class_type = function.Class;
        auto self_llvm_type = self_class_type->GenIR(*this);

        self_class_type->ForEachMember(
            [this, self_llvm_type, self_pointer](auto index, auto& member)
            {
                auto type = member.Info.GenIRType(*this);
                auto value = llvm::Constant::getNullValue(type);
                auto pointer = CreateStructGEP(self_llvm_type, self_pointer, index);

                CreateStore(value, pointer);
            });

        // TODO: set virtual pointers

        for (auto& initializer : function.Initializers)
        {
            std::vector<Field> argument_fields;
            std::vector<ValuePtr> argument_values;
            for (auto& argument : initializer.Arguments)
            {
                auto argument_value = argument->GenVal(
                    *this,
                    initializer.Arguments.size() == 1 ? self_class_type : nullptr);
                argument_fields.emplace_back(argument_value->AsField());
                argument_values.emplace_back(std::move(argument_value));
            }

            if (initializer.Name == "create")
            {
                Assert(self_class_type->HasParentClass(), "parent initialization from orphan class");
                auto parent_class = self_class_type->GetParentClass();

                auto constructors = parent_class->GetConstructors(parent_class);
                auto candidate = FindFunction(constructors, argument_fields, *self, false);
                Assert(candidate.has_value(), "no suitable candidate");

                CreateCall(*candidate, std::move(argument_values), Value::CreateL(self->GetType(), self_pointer, self->IsMutable()));
                continue;
            }

            auto index = self_class_type->GetMemberIndex(initializer.Name);
            auto member = self_class_type->GetMember(index);
            auto member_pointer = CreateStructGEP(self_llvm_type, self_pointer, index);

            ValuePtr value;
            if (initializer.Value)
            {
                value = initializer.Value->GenVal(*this, member.GetType());
            }

            if (member.IsReference())
            {
                Assert(initializer.Arguments.empty(), "cannot construct reference");
                Assert(value != nullptr, "missing initializer value");
                Assert(value->IsReference(), "reference from rvalue");
                Assert(
                    value->GetType() == member.GetType()
                        || (value->GetType()->IsClass()
                            && As<ClassType>(value->GetType())
                                   ->InheritsFrom(member.GetType())),
                    "reference type mismatch");
                Assert(!member.IsMutable() || value->IsMutable(), "reference mutability violation");

                CreateStore(value->GetPointer(), member_pointer);
            }
            else
            {
                if (member.GetType()->IsClass())
                {
                    const auto member_self = Value::CreateL(member.GetType(), member_pointer, true);

                    const auto member_class_type = As<ClassType>(member.GetType());
                    const auto constructors = member_class_type->GetConstructors(member_class_type);

                    if (value)
                    {
                        if (const auto candidate = FindFunction(
                                constructors,
                                { value->AsField() },
                                member_self->AsField(),
                                true))
                        {
                            CreateCall(*candidate, { std::move(value) }, member_self);
                        }
                        else
                        {
                            Assert(!value->IsReference(), "illegal implicit copy");

                            value = CreateCast(std::move(value), member.GetType(), true);
                            CreateStore(value->Load(*this), member_pointer);
                        }
                    }
                    else if (constructors.empty())
                    {
                        Assert(argument_values.empty(), "illegal arguments for implicit default constructor");

                        CreateStore(llvm::Constant::getNullValue(member.GenIRType(*this)), member_pointer);
                    }
                    else
                    {
                        const auto candidate = FindFunction(
                            constructors,
                            argument_fields,
                            member_self->AsField(),
                            false);
                        Assert(candidate.has_value(), "no suitable candidate");

                        CreateCall(*candidate, std::move(argument_values), member_self);
                    }
                }
                else
                {
                    if (!value)
                    {
                        Assert(argument_values.empty(), "cannot construct non-class value");

                        value = Value::CreateR(
                            member.GetType(),
                            llvm::Constant::getNullValue(member.GenIRType(*this)));
                    }
                    else if (member.GetType())
                    {
                        value = CreateCast(std::move(value), member.GetType(), true);
                    }

                    CreateStore(value->Load(*this), member_pointer);
                }
            }
        }
    }

    function.Content->Gen(*this);
    PopFrame();

    m_DebugBuilder.EndFunction();

    for (auto& block : *callee)
    {
        if (block.getTerminator())
        {
            continue;
        }

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

    if (verifyFunction(*callee, &llvm::errs()))
    {
        callee->print(llvm::errs());
        Error("function has errors");
    }

    return reference;
}

llvm::Value* llove::Builder::GenParameters(
    llvm::Function* parent,
    const std::vector<Parameter>& parameters,
    const std::pair<
        bool,
        std::string>& variadic,
    const std::optional<Field>& self)
{
    auto iterator = parent->arg_begin();
    auto index = 1u;

    llvm::Value* self_pointer = nullptr;
    if (self)
    {
        const auto argument = iterator++;
        argument->setName("self");
        self_pointer = argument;

        auto storage = Value::CreateL(self->GetType(), argument, self->IsMutable());

        m_DebugBuilder.CreateParameter(*this, "self", index++, storage);
        SetValue("self", std::move(storage));
    }

    for (auto& parameter : parameters)
    {
        const auto argument = iterator++;
        argument->setName(parameter.Name);

        ValuePtr storage;
        if (parameter.Info.IsReference())
        {
            storage = Value::CreateL(
                parameter.Info.GetType(),
                argument,
                parameter.Info.IsMutable());
        }
        else if (parameter.Info.GetType()->IsClass())
        {
            const auto pointer = CreateAlloca(parameter.Info.GetType()->GenIR(*this), parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(
                parameter.Info.GetType(),
                pointer,
                parameter.Info.IsMutable());

            PushDestructor(pointer, As<ClassType>(parameter.Info.GetType()));
        }
        else if (parameter.Info.IsMutable())
        {
            const auto pointer = CreateAlloca(parameter.Info.GetType()->GenIR(*this), parent);
            m_LLVMBuilder.CreateStore(argument, pointer);

            storage = Value::CreateL(
                parameter.Info.GetType(),
                pointer,
                parameter.Info.IsMutable());
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

    return self_pointer;
}
