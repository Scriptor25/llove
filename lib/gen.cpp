#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

void llove::DefinitionGlobal::Gen(Builder &builder) const
{
    builder.GenFunction(
        {
            .Interface = m_Interface,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .VarArg = m_VarArg,
            .Result = m_Result,
            .Content = m_Content.get(),
        }
    );
}

void llove::ClassDefinitionGlobal::Gen(Builder &builder) const
{
    std::vector<Field> parameters;
    for (auto &[info_, name_] : m_Parameters)
        parameters.emplace_back(info_);
    const auto class_function = m_ClassType->GetFunction(m_Name, m_Mutable, parameters, m_VarArg, m_Result);

    Assert(class_function != nullptr, "class function prototype mismatch");

    builder.GenFunction(
        {
            .ClassType = m_ClassType,
            .Mutable = m_Mutable,
            .Expose = class_function->Expose,
            .Name = m_Name,
            .Parameters = m_Parameters,
            .VarArg = m_VarArg,
            .Result = m_Result,
            .Content = m_Content.get(),
        }
    );
}

void llove::ClassGlobal::Gen(Builder &builder) const
{
    if (m_Opaque)
        return;

    std::vector<ClassFieldReference> class_fields;
    for (auto &[info_, name_] : m_Fields)
        class_fields.emplace_back(info_, name_);
    m_Type->SetFields(builder, std::move(class_fields));

    std::vector<ClassFunctionReference> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info_, name_] : function.Parameters)
            parameters.emplace_back(info_);
        class_functions.emplace_back(
            ClassFunctionReference
            {
                .Expose = function.Expose,
                .Mutable = function.Mutable,
                .Name = function.Name,
                .Parameters = std::move(parameters),
                .VarArg = function.VarArg,
                .Result = function.Result,
            }
        );
    }

    m_Type->SetFunctions(std::move(class_functions));

    for (auto &[
             expose_,
             mutable_,
             name_,
             parameters_,
             vararg_,
             result_,
             content_
         ] : m_Functions)
    {
        builder.GenFunction(
            {
                .ClassType = m_Type,
                .Mutable = mutable_,
                .Expose = expose_,
                .Name = name_,
                .Parameters = parameters_,
                .VarArg = vararg_,
                .Result = result_,
            }
        );
    }

    for (auto &[
             expose_,
             mutable_,
             name_,
             parameters_,
             vararg_,
             result_,
             content_
         ] : m_Functions)
    {
        builder.GenFunction(
            {
                .ClassType = m_Type,
                .Mutable = mutable_,
                .Expose = expose_,
                .Name = name_,
                .Parameters = parameters_,
                .VarArg = vararg_,
                .Result = result_,
                .Content = content_.get(),
            }
        );
    }
}

void llove::ScopeStatement::Gen(Builder &builder) const
{
    builder.PushFrame();
    for (auto &ptr : m_Content)
        ptr->Gen(builder);
    builder.PopFrame();
}

void llove::ForStatement::Gen(Builder &builder) const
{
    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    const auto loop_block = builder.CreateBlock("loop", parent);
    const auto end_block = builder.CreateBlock("end");

    auto use_end = false;

    builder.PushFrame();

    if (m_Prefix)
        m_Prefix->Gen(builder);
    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);
    if (m_Condition)
    {
        const auto condition = m_Condition->GenVal(builder, builder.GetTypes().GetInteger(false, 1));
        builder.CreateBranch(condition, loop_block, end_block);
        use_end = true;
    }
    else
    {
        builder.CreateBranch(loop_block);
    }

    builder.SetInsertPoint(loop_block);
    m_Content->Gen(builder);
    if (builder.NoTerminator())
    {
        if (m_Suffix)
            m_Suffix->Gen(builder);
        builder.CreateBranch(head_block);
    }

    if (use_end)
    {
        end_block->insertInto(parent);
        builder.SetInsertPoint(end_block);
    }
    else
    {
        end_block->deleteValue();
        builder.ClearInsertPoint();
    }

    builder.PopFrame();
}

void llove::ForEachStatement::Gen(Builder &builder) const
{
    Error("not yet implemented");
}

void llove::IfStatement::Gen(Builder &builder) const
{
    const auto parent = builder.GetParent();
    const auto then_block = builder.CreateBlock("then", parent);
    const auto else_block = builder.CreateBlock("else", parent);
    const auto end_block = builder.CreateBlock("end");

    auto use_end = false;

    const auto condition = m_Condition->GenVal(builder, builder.GetTypes().GetInteger(false, 1));
    builder.CreateBranch(condition, then_block, else_block);

    builder.SetInsertPoint(then_block);
    m_Then->Gen(builder);
    if (builder.NoTerminator())
    {
        builder.CreateBranch(end_block);
        use_end = true;
    }

    builder.SetInsertPoint(else_block);
    if (m_Else)
    {
        m_Else->Gen(builder);
    }
    if (builder.NoTerminator())
    {
        builder.CreateBranch(end_block);
        use_end = true;
    }

    if (use_end)
    {
        end_block->insertInto(parent);
        builder.SetInsertPoint(end_block);
    }
    else
    {
        end_block->deleteValue();
        builder.ClearInsertPoint();
    }
}

void llove::LetStatement::Gen(Builder &builder) const
{
    Assert(m_Info.Type != nullptr || m_Value != nullptr, "missing type or value");

    auto value = m_Value ? m_Value->GenVal(builder, m_Info.Type) : nullptr;
    auto type = m_Info.Type ? m_Info.Type : value->GetType();

    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
        arguments.emplace_back(argument->GenVal(builder, nullptr));

    ValuePtr storage;
    if (m_Info.Reference)
    {
        Assert(arguments.empty(), "cannot construct reference");
        Assert(value != nullptr, "missing value");
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(type == value->GetType(), "reference type mismatch");
        Assert(!m_Info.Mutable || value->IsMutable(), "reference mutability violation");

        storage = Value::CreateL(std::move(type), value->GetPointer(), m_Info.Mutable);
    }
    else
    {
        const auto pointer = builder.CreateAlloca(type);

        if (type && type->GetId() == TypeId_Class)
        {
            const Field self
            {
                .Mutable = m_Info.Mutable,
                .Reference = true,
                .Type = type,
            };

            const auto class_type = As<ClassType>(type);
            const auto constructors = class_type->GetConstructors();

            if (arguments.empty())
            {
                std::vector<Field> argument_fields;
                if (value)
                    argument_fields.emplace_back(value->AsField());

                if (auto candidate = builder.FindFunction(constructors, argument_fields, class_type, self))
                {
                    // construct from single argument value
                    Error("TODO");
                }
                else
                {
                    if (value)
                    {
                        value = builder.CreateCast(std::move(value), type);
                    }
                    else
                    {
                        const auto null = llvm::Constant::getNullValue(type->Gen(builder));
                        value = Value::CreateR(std::move(type), null);
                    }
                    builder.CreateStore(pointer, value);
                }
            }
            else
            {
                std::vector<Field> argument_fields;
                for (const auto &argument : arguments)
                    argument_fields.emplace_back(argument->AsField());

                const auto candidate = builder.FindFunction(
                    constructors,
                    argument_fields,
                    class_type,
                    self);
                Assert(candidate != nullptr, "no suitable candidate");

                // construct from arguments
                Error("TODO");
            }
        }
        else
        {
            if (!value)
            {
                Assert(arguments.empty(), "cannot construct non-class value");
                Assert(type != nullptr, "missing type");

                const auto null = llvm::Constant::getNullValue(type->Gen(builder));
                value = Value::CreateR(std::move(type), null);
            }
            else
            {
                value = builder.CreateCast(std::move(value), type);
            }

            builder.CreateStore(pointer, value);
        }

        storage = Value::CreateL(std::move(type), pointer, m_Info.Mutable);
    }

    builder.SetValue(m_Name, std::move(storage));
}

void llove::YieldStatement::Gen(Builder &builder) const
{
    if (!m_Value)
    {
        builder.CreateRetVoid();
        return;
    }

    auto &result = builder.GetResult();
    auto value = m_Value->GenVal(builder, result.Type);

    builder.CreateRet(result.GenCast(builder, std::move(value), true));
}

void llove::Expression::Gen(Builder &builder) const
{
    (void) GenVal(builder, nullptr);
}

llove::CalleeInfo llove::Expression::GenCallee(Builder &builder) const
{
    const auto value = GenVal(builder, nullptr);

    const auto pointer_type = As<PointerType>(value->GetType());
    const auto is_function_pointer = pointer_type
                                     && !pointer_type->IsOpaque()
                                     && pointer_type->GetBase()->GetId() == TypeId_Function;

    Assert(is_function_pointer, "not a function pointer");

    return {
        .Candidates = {
            FunctionReference
            {
                .Expose = false,
                .Name = {},
                .Type = As<FunctionType>(pointer_type->GetBase()),
                .Callee = value->Load(builder),
            }
        }
    };
}

llove::ValuePtr llove::NullExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    PointerType::Ptr type;
    if (m_Type)
        type = builder.GetTypes().GetPointer(m_Type, false);
    else if (auto pointer_type = As<PointerType>(expect))
        type = std::move(pointer_type);
    else
        type = builder.GetTypes().GetPointer(false);
    const auto value = llvm::ConstantPointerNull::get(type->Gen(builder));
    return Value::CreateR(std::move(type), value);
}

llove::ValuePtr llove::IntExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    auto type = m_Type;
    if (!type)
        type = As<IntegerType>(expect);
    if (!type)
        type = builder.GetTypes().GetInteger(false, 64);
    const auto value = llvm::ConstantInt::get(type->Gen(builder), m_Value, type->IsSigned());
    return Value::CreateR(std::move(type), value);
}

llove::ValuePtr llove::StringExpression::GenVal(Builder &builder, TypePtr expect) const
{
    const auto value = builder.CreateGlobalString(m_Value);
    return Value::CreateR(builder.GetTypes().GetPointer(builder.GetTypes().GetInteger(true, 8), false), value);
}

llove::ValuePtr llove::RangeExpression::GenVal(Builder &builder, TypePtr expect) const
{
    Error("not yet implemented");
}

llove::ValuePtr llove::ArrayExpression::GenVal(Builder &builder, TypePtr expect) const
{
    Error("not yet implemented");
}

llove::ValuePtr llove::StructExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    const auto type = m_Type ? m_Type : As<StructType>(expect);
    Assert(type != nullptr, "untyped struct expression");

    const auto pointer = builder.CreateAlloca(type);
    builder.CreateStore(pointer, llvm::Constant::getNullValue(type->Gen(builder)));

    for (auto &[key_, value_] : m_Values)
    {
        const auto index = type->GetFieldIndex(key_);
        auto &field = type->GetField(index);

        auto value = value_->GenVal(builder, field.Type);
        const auto llvm_value = field.GenCast(builder, std::move(value), true);

        const auto element_pointer = builder.CreateStructGEP(type, pointer, index);
        builder.CreateStore(element_pointer, llvm_value);
    }

    return Value::CreateL(type, pointer, false);
}

llove::ValuePtr llove::SymbolExpression::GenVal(Builder &builder, TypePtr expect) const
{
    // TODO: if no symbol with name exists, return single function with name if exists

    auto value = builder.GetValue(m_Name);
    Assert(value != nullptr, "undefined symbol name '{}'", m_Name);
    return value;
}

llove::CalleeInfo llove::SymbolExpression::GenCallee(Builder &builder) const
{
    // TODO: if symbol with name exists, add to candidates

    auto candidates = builder.GetFunctions(m_Name);
    Assert(!candidates.empty(), "undefined symbol name '{}'", m_Name);
    return { .Candidates = std::move(candidates) };
}

llove::ValuePtr llove::BinaryExpression::GenVal(Builder &builder, TypePtr expect) const
{
    static const std::map<std::string_view, const char *> assign
    {
        { "+=", "+" },
        { "-=", "-" },
        { "*=", "*" },
        { "/=", "/" },
        { "%=", "%" },
        { "&=", "&" },
        { "|=", "|" },
        { "^=", "^" },
        { "<<=", "<<" },
        { ">>=", ">>" },
    };

    auto left = m_Left->GenVal(builder, nullptr);
    auto right = m_Right->GenVal(builder, left->GetType());

    if (const auto operator_ = builder.FindOperator(m_Operator, left->AsField(), right->AsField()))
        return (*operator_)(builder, std::move(left), std::move(right));

    if (assign.contains(m_Operator))
        if (const auto operator_ = builder.FindOperator(assign.at(m_Operator), left->AsField(), right->AsField()))
        {
            const auto value = (*operator_)(builder, left, std::move(right));
            left->Store(builder, value);
            return left;
        }

    Error("undefined binary operator {} {} {}", left->GetType(), m_Operator, right->GetType());
}

llove::ValuePtr llove::UnaryExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    auto operand = m_Operand->GenVal(builder, expect);

    if (const auto operator_ = builder.FindOperator(m_Operator, operand->AsField(), m_Suffix))
        return (*operator_)(builder, std::move(operand));

    Error(
        "undefined unary operator {}{}{}",
        m_Suffix ? std::string{} : m_Operator,
        operand->GetType(),
        m_Suffix ? m_Operator : std::string{});
}

llove::ValuePtr llove::CallExpression::GenVal(Builder &builder, TypePtr expect) const
{
    auto [functions, self] = m_Callee->GenCallee(builder);

    std::vector<ValuePtr> arguments;
    std::vector<Field> argument_fields;
    for (auto &argument : m_Arguments)
    {
        auto value = argument->GenVal(builder, nullptr);
        arguments.emplace_back(value);
        argument_fields.emplace_back(value->AsField());
    }

    const auto candidate = builder.FindFunction(
        functions,
        argument_fields,
        self != nullptr,
        self ? self->AsField() : Field{});
    Assert(candidate != nullptr, "no suitable candidate");

    return builder.CreateCall(candidate->Type, candidate->Callee, std::move(arguments), std::move(self));
}

llove::ValuePtr llove::MemberExpression::GenVal(Builder &builder, TypePtr expect) const
{
    const auto value = m_Value->GenVal(builder, nullptr);
    const auto type = value->GetType();

    unsigned index;
    Field element;

    if (const auto struct_type = As<StructType>(type))
    {
        index = struct_type->GetFieldIndex(m_Member);
        element = struct_type->GetField(index);
    }
    else if (const auto class_type = As<ClassType>(type))
    {
        // TODO: check if field is accessible
        // TODO: if no field with name exists, return single function with name if exists and is accessible
        index = class_type->GetFieldIndex(m_Member);
        element = class_type->GetField(index);
    }
    else
    {
        Error("not yet implemented");
    }

    if (value->IsReferenceable())
    {
        const auto result = builder.CreateStructGEP(type, value->GetPointer(), index);

        if (element.Reference)
        {
            const auto pointer = builder.CreateLoad(result, builder.GetTypes().GetPointer(false));
            return Value::CreateL(element.Type, pointer, element.Mutable);
        }

        return Value::CreateL(element.Type, result, value->IsMutable() && element.Mutable);
    }

    const auto result = builder.CreateExtractValue(value->Load(builder), index);

    if (element.Reference)
    {
        const auto pointer = builder.CreateLoad(result, builder.GetTypes().GetPointer(false));
        return Value::CreateL(element.Type, pointer, element.Mutable);
    }

    return Value::CreateR(element.Type, result);
}

llove::CalleeInfo llove::MemberExpression::GenCallee(Builder &builder) const
{
    // TODO: if field with name exists and is accessible, add to candidates

    auto value = m_Value->GenVal(builder, nullptr);
    return { .Candidates = builder.GetFunctions(m_Member, value->AsField()), .Self = std::move(value) };
}

llove::ValuePtr llove::SubscriptExpression::GenVal(Builder &builder, const TypePtr expect) const
{
    auto value = m_Value->GenVal(builder, expect ? builder.GetTypes().GetPointer(expect, false) : nullptr);
    const auto index = m_Index->GenVal(builder, nullptr);

    switch (value->GetType()->GetId())
    {
    case TypeId_Pointer:
        return builder.CreatePointerElement(value, index);
    case TypeId_Array:
        return builder.CreateArrayElement(std::move(value), index);
    default:
        Error("subscript on non-pointer and non-array value of type {}", value->GetType());
    }
}
