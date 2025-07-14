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
    const auto class_type = builder.GetTypes().GetClass(m_ClassName);
    std::vector<Field> parameters;
    for (auto &[info_, name_] : m_Parameters)
        parameters.emplace_back(info_);
    const auto class_function = class_type->GetFunction(m_Name, m_Mutable, parameters, m_VarArg, m_Result);

    Assert(class_function != nullptr, "class function prototype mismatch");

    builder.GenFunction(
        {
            .ClassName = m_ClassName,
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

    const auto class_type = builder.GetTypes().GetClass(m_Name);

    std::vector<ClassField> class_fields;
    for (auto &[info_, name_] : m_Fields)
        class_fields.emplace_back(info_, name_);
    class_type->SetFields(builder, std::move(class_fields));

    std::vector<ClassFunctionInfo> class_functions;
    for (auto &function : m_Functions)
    {
        std::vector<Field> parameters;
        for (const auto &[info_, name_] : function.Parameters)
            parameters.emplace_back(info_);
        class_functions.emplace_back(
            ClassFunctionInfo
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
    class_type->SetFunctions(std::move(class_functions));

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
                .ClassName = m_Name,
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
                .ClassName = m_Name,
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
    builder.StackPush();
    for (auto &ptr : m_Content)
        ptr->Gen(builder);
    builder.StackPop();
}

void llove::ForStatement::Gen(Builder &builder) const
{
    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    const auto loop_block = builder.CreateBlock("loop", parent);
    const auto end_block = builder.CreateBlock("end");

    auto use_end = false;

    builder.StackPush();

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
    if (!builder.GetInsertBlock()->getTerminator())
    {
        if (m_Suffix)
            m_Suffix->Gen(builder);
        builder.CreateBranch(head_block);
    }

    builder.StackPop();

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
    if (!builder.GetInsertBlock()->getTerminator())
    {
        builder.CreateBranch(end_block);
        use_end = true;
    }

    builder.SetInsertPoint(else_block);
    if (m_Else)
    {
        m_Else->Gen(builder);
    }
    if (!builder.GetInsertBlock()->getTerminator())
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
    auto value = m_Value ? m_Value->GenVal(builder, m_Info.Type) : nullptr;
    Assert(m_Info.Type != nullptr || value != nullptr, "missing type or value");
    const auto type = m_Info.Type ? m_Info.Type : value->GetType();

    ValuePtr storage;
    if (m_Info.Reference)
    {
        Assert(value != nullptr, "missing value");
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(type == value->GetType(), "reference type mismatch");
        Assert(!m_Info.Mutable || value->IsMutable(), "reference mutability violation");

        storage = Value::CreateL(type, value->GetPointer(), m_Info.Mutable);
    }
    else
    {
        if (!value)
        {
            Assert(type != nullptr, "missing type");
            const auto empty = llvm::Constant::getNullValue(type->Gen(builder));
            value = Value::CreateR(type, empty);
        }
        else if (type)
        {
            value = builder.CreateCast(value, type);
        }

        const auto pointer = builder.CreateAlloca(builder.GetParent(), type);
        builder.CreateStore(pointer, value->Load(builder));

        storage = Value::CreateL(type, pointer, m_Info.Mutable);
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

    auto [mutable_, reference_, type_] = builder.GetResult();
    auto value = m_Value->GenVal(builder, type_);

    llvm::Value *llvm_value;
    if (reference_)
    {
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(type_ == value->GetType(), "reference type mismatch");
        Assert(!mutable_ || value->IsMutable(), "reference mutability violation");

        llvm_value = value->GetPointer();
    }
    else
    {
        value = builder.CreateCast(value, type_);

        llvm_value = value->Load(builder);
    }

    builder.CreateRet(llvm_value);
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

    const auto callee = value->Load(builder);

    return {
        .Candidates = {
            FunctionReference
            {
                .Expose = false,
                .Name = {},
                .Type = As<FunctionType>(pointer_type->GetBase()),
                .Callee = callee,
            }
        }
    };
}

llove::ValuePtr llove::NullExpression::GenVal(Builder &builder, TypePtr expect) const
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

llove::ValuePtr llove::IntExpression::GenVal(Builder &builder, TypePtr expect) const
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

llove::ValuePtr llove::StructExpression::GenVal(Builder &builder, TypePtr expect) const
{
    const auto type = m_Type ? m_Type : As<StructType>(expect);
    Assert(type != nullptr, "untyped struct expression");

    const auto aggregate_type = type->Gen(builder);
    llvm::Value *aggregate = llvm::ConstantStruct::get(aggregate_type);

    for (auto &[key_, value_] : m_Values)
    {
        const auto index = type->GetFieldIndex(key_);
        auto &[mutable_, reference_, type_] = type->GetField(index);

        const auto value = value_->GenVal(builder, type_);

        llvm::Value *llvm_value;
        if (reference_)
        {
            Assert(value->IsReferenceable(), "reference from rvalue");
            Assert(type_ == value->GetType(), "reference type mismatch");
            Assert(!mutable_ || value->IsMutable(), "reference mutability violation");

            llvm_value = value->GetPointer();
        }
        else
        {
            llvm_value = value->Load(builder);
        }

        builder.CreateInsertValue(aggregate, llvm_value, index);
    }

    return Value::CreateR(type, aggregate);
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

    if (const auto operator_ = builder.GetOperator(m_Operator, left->AsField(), right->AsField()))
        return (*operator_)(builder, std::move(left), std::move(right));

    if (assign.contains(m_Operator))
        if (const auto operator_ = builder.GetOperator(assign.at(m_Operator), left->AsField(), right->AsField()))
        {
            const auto value = (*operator_)(builder, left, std::move(right));
            left->Store(builder, value->Load(builder));
            return left;
        }

    Error("undefined binary operator {} {} {}", left->GetType(), m_Operator, right->GetType());
}

llove::ValuePtr llove::UnaryExpression::GenVal(Builder &builder, TypePtr expect) const
{
    auto operand = m_Operand->GenVal(builder, expect);

    if (const auto operator_ = builder.GetOperator(m_Operator, operand->AsField(), m_Suffix))
        return (*operator_)(builder, std::move(operand));

    Error(
        "undefined unary operator {}{}{}",
        m_Suffix ? std::string{} : m_Operator,
        operand->GetType(),
        m_Suffix ? m_Operator : std::string{});
}

llove::ValuePtr llove::CallExpression::GenVal(Builder &builder, TypePtr expect) const
{
    auto [
        candidates_,
        self_
    ] = m_Callee->GenCallee(builder);

    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
        arguments.emplace_back(argument->GenVal(builder, nullptr));

    const auto has_self = self_ != nullptr;

    auto lowest_error = ~0u;
    const FunctionReference *callee = nullptr;

    for (const auto &candidate : candidates_)
    {
        const auto type = candidate.Type;

        if (type->HasSelf() != has_self)
            continue;

        auto error = 0u;

        if (has_self)
        {
            auto &function_self = type->GetSelf();
            if (function_self.Type != self_->GetType())
                continue;
            if (function_self.Mutable && (!self_->IsReferenceable() || !self_->IsMutable()))
                continue;

            if (!self_->IsReferenceable())
                error += 20u;
        }

        if (type->GetParameterCount() > arguments.size())
            continue;
        if (!type->IsVarArg() && type->GetParameterCount() < arguments.size())
            continue;

        if (type->GetParameterCount() != arguments.size())
            error += 2u;

        unsigned i;
        for (i = 0; i < type->GetParameterCount(); ++i)
        {
            auto &[mutable_, reference_, type_] = type->GetParameter(i);
            const auto &argument = arguments.at(i);
            if (reference_)
            {
                if (type_ != argument->GetType())
                    break;
                if (mutable_ && (!argument->IsReferenceable() || !argument->IsMutable()))
                    break;

                if (!argument->IsReferenceable())
                    error += 10u;
            }
            else if (type_ != argument->GetType())
            {
                if (!builder.IsCastable(argument->IsMutable(), argument->GetType(), type_))
                    break;

                error += 5u;
            }
        }
        if (i < type->GetParameterCount())
            continue;

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        callee = &candidate;
    }

    Assert(callee != nullptr, "no suitable callee candidate");

    std::vector<llvm::Value *> llvm_arguments;

    if (has_self)
    {
        if (!self_->IsReferenceable())
        {
            const auto storage = builder.CreateAlloca(builder.GetParent(), self_->GetType());
            builder.CreateStore(storage, self_->Load(builder));
            self_ = Value::CreateL(self_->GetType(), storage, false);
        }
        llvm_arguments.emplace_back(self_->GetPointer());
    }

    unsigned i;
    // ReSharper disable once CppDFANullDereference
    for (i = 0; i < callee->Type->GetParameterCount(); ++i)
    {
        auto &parameter = callee->Type->GetParameter(i);
        const auto &argument = arguments.at(i);

        llvm_arguments.emplace_back(parameter.Gen(builder, argument));
    }
    for (; i < arguments.size(); ++i)
        llvm_arguments.emplace_back(arguments.at(i)->Load(builder));

    const auto result_value = builder.CreateCall(callee->Type, callee->Callee, llvm_arguments);

    auto &[mutable_, reference_, type_] = callee->Type->GetResult();
    if (reference_)
        return Value::CreateL(type_, result_value, mutable_);
    return Value::CreateR(type_, result_value);
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
        const auto element_pointer = builder.CreateStructGEP(type, value->GetPointer(), index);

        auto result = Value::CreateL(element.Type, element_pointer, value->IsMutable() && element.Mutable);

        if (element.Reference)
        {
            auto pointer = result->GetPointer();
            pointer = builder.CreateLoad(pointer, builder.GetTypes().GetPointer(false));
            result = Value::CreateL(element.Type, pointer, element.Mutable);
        }

        return result;
    }

    Error("not yet implemented");
}

llove::CalleeInfo llove::MemberExpression::GenCallee(Builder &builder) const
{
    // TODO: if field with name exists and is accessible, add to candidates

    auto value = m_Value->GenVal(builder, nullptr);
    return { .Candidates = builder.GetFunctions(m_Member, value->AsField()), .Self = std::move(value) };
}

llove::ValuePtr llove::SubscriptExpression::GenVal(Builder &builder, TypePtr expect) const
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

llove::ValuePtr llove::CreateExpression::GenVal(Builder &builder, TypePtr expect) const
{
    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
        arguments.emplace_back(argument->GenVal(builder, nullptr));

    auto lowest_error = ~0u;
    const ClassFunctionInfo *create = nullptr;

    for (const auto candidates = m_ClassType->GetCreates();
         const auto candidate : candidates)
    {
        if (candidate->Parameters.size() > arguments.size())
            continue;
        if (!candidate->VarArg && candidate->Parameters.size() < arguments.size())
            continue;

        auto error = 0u;

        if (candidate->Parameters.size() != arguments.size())
            error += 2u;

        unsigned i;
        for (i = 0; i < candidate->Parameters.size(); ++i)
        {
            auto &[mutable_, reference_, type_] = candidate->Parameters.at(i);
            const auto &argument = arguments.at(i);
            if (reference_)
            {
                if (type_ != argument->GetType())
                    break;
                if (mutable_ && (!argument->IsReferenceable() || !argument->IsMutable()))
                    break;

                if (!argument->IsReferenceable())
                    error += 10u;
            }
            else if (type_ != argument->GetType())
            {
                if (!builder.IsCastable(argument->IsMutable(), argument->GetType(), type_))
                    break;

                error += 5u;
            }
        }
        if (i < candidate->Parameters.size())
            continue;

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        create = candidate;
    }

    Assert(create != nullptr, "no suitable create candidate");

    std::vector<llvm::Value *> llvm_arguments;

    const auto self = builder.CreateAlloca(builder.GetParent(), m_ClassType);
    llvm_arguments.emplace_back(self);

    unsigned i;
    // ReSharper disable once CppDFANullDereference
    for (i = 0; i < create->Parameters.size(); ++i)
    {
        auto &parameter = create->Parameters.at(i);
        const auto &argument = arguments.at(i);

        llvm_arguments.emplace_back(parameter.Gen(builder, argument));
    }
    for (; i < arguments.size(); ++i)
    {
        llvm_arguments.emplace_back(arguments.at(i)->Load(builder));
    }

    std::vector<Parameter> parameters;
    for (auto &parameter : create->Parameters)
        parameters.emplace_back(parameter, std::string{});
    const auto callee = builder.GenFunction(
        {
            .ClassName = m_ClassType->GetName(),
            .Mutable = create->Mutable,
            .Expose = create->Expose,
            .Name = create->Name,
            .Parameters = std::move(parameters),
            .VarArg = create->VarArg,
            .Result = create->Result,
        });
    builder.CreateCall(callee, llvm_arguments);

    return Value::CreateL(m_ClassType, self, true);
}
