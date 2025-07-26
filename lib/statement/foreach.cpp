#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ForEachStatement::ForEachStatement(
    const bool mutable_,
    const bool reference,
    std::string name,
    ExpressionPtr range,
    StatementPtr content)
    : m_Mutable(mutable_),
      m_Reference(reference),
      m_Name(std::move(name)),
      m_Range(std::move(range)),
      m_Content(std::move(content))
{
}

void llove::ForEachStatement::Gen(Builder &builder) const
{
    builder.PushFrame();

    const auto range = m_Range->GenVal(builder, nullptr);
    const auto type = range->GetType();

    ValuePtr begin;
    ValuePtr end;

    switch (type->GetId())
    {
    case TypeId_Array:
    {
        const auto array_type = As<ArrayType>(type);
        const auto base_type = array_type->GetBase();
        auto iterator_type = builder.GetTypes().GetPointer(base_type, range->IsMutable());

        if (range->IsReferenceable())
        {
            const auto pointer = range->GetPointer();
            begin = Value::CreateR(std::move(iterator_type), pointer);
        }
        else
        {
            const auto pointer = builder.CreateAlloca(type);
            builder.CreateStore(pointer, range);
            begin = Value::CreateR(std::move(iterator_type), pointer);
        }

        end = builder.CreatePointerOffset(begin, array_type->GetSize());
        break;
    }
    case TypeId_Struct:
    {
        const auto struct_type = As<StructType>(type);
        Assert(struct_type->HasField("begin"), "struct is missing field 'begin'");
        Assert(struct_type->HasField("end"), "struct is missing field 'end'");

        const auto begin_index = struct_type->GetFieldIndex("begin");
        const auto end_index = struct_type->GetFieldIndex("end");

        auto &[
            begin_mutable,
            begin_reference,
            begin_type
        ] = struct_type->GetField(begin_index);
        auto &[
            end_mutable,
            end_reference,
            end_type
        ] = struct_type->GetField(end_index);

        if (range->IsReferenceable())
        {
            auto begin_pointer = builder.CreateStructGEP(struct_type, range->GetPointer(), begin_index);
            if (begin_reference)
            {
                begin_pointer = builder.CreateLoad(
                    begin_pointer,
                    builder.GetTypes().GetPointer(begin_type, begin_mutable));
                begin = Value::CreateL(begin_type, begin_pointer, begin_mutable);
            }
            else
            {
                begin = Value::CreateL(begin_type, begin_pointer, range->IsMutable() && begin_mutable);
            }

            auto end_pointer = builder.CreateStructGEP(struct_type, range->GetPointer(), end_index);
            if (end_reference)
            {
                end_pointer = builder.CreateLoad(
                    end_pointer,
                    builder.GetTypes().GetPointer(end_type, end_mutable));
                end = Value::CreateL(end_type, end_pointer, end_mutable);
            }
            else
            {
                end = Value::CreateL(end_type, end_pointer, range->IsMutable() && end_mutable);
            }
        }
        else
        {
            auto begin_value = builder.CreateExtractValue(range, begin_index);
            if (begin_reference)
            {
                begin_value = builder.CreateLoad(
                    begin_value,
                    builder.GetTypes().GetPointer(begin_type, begin_mutable));
                begin = Value::CreateL(begin_type, begin_value, begin_mutable);
            }
            else
            {
                begin = Value::CreateR(begin_type, begin_value);
            }

            auto end_value = builder.CreateExtractValue(range, end_index);
            if (end_reference)
            {
                end_value = builder.CreateLoad(
                    end_value,
                    builder.GetTypes().GetPointer(end_type, end_mutable));
                end = Value::CreateL(end_type, end_value, end_mutable);
            }
            else
            {
                end = Value::CreateR(end_type, end_value);
            }
        }

        if (begin_type->IsFunction())
        {
            const auto fn_type = As<FunctionType>(begin_type);
            Assert(!fn_type->IsVarArg(), "invalid vararg");
            Assert(fn_type->GetParameterCount() == 0, "invalid parameter count");

            begin = builder.CreateCall(fn_type, begin->Load(builder), {}, {});
        }

        if (end_type->IsFunction())
        {
            const auto fn_type = As<FunctionType>(end_type);
            Assert(!fn_type->IsVarArg(), "invalid vararg");
            Assert(fn_type->GetParameterCount() == 0, "invalid parameter count");

            end = builder.CreateCall(fn_type, end->Load(builder), {}, {});
        }

        break;
    }
    case TypeId_Range:
    {
        const auto range_type = As<RangeType>(type);
        const auto iterator_type = range_type->GetEntry();

        if (range->IsReferenceable())
        {
            const auto begin_pointer = builder.CreateStructGEP(range_type, range->GetPointer(), 0);
            const auto end_pointer = builder.CreateStructGEP(range_type, range->GetPointer(), 1);

            begin = Value::CreateL(iterator_type, begin_pointer, false);
            end = Value::CreateL(iterator_type, end_pointer, false);
        }
        else
        {
            const auto value = range->Load(builder);

            begin = Value::CreateR(iterator_type, builder.CreateExtractValue(value, 0));
            end = Value::CreateR(iterator_type, builder.CreateExtractValue(value, 1));
        }

        break;
    }
    case TypeId_Class:
    {
        const auto class_type = As<ClassType>(type);

        auto begin_function = builder.FindFunction(
            class_type->GetFunctions("begin"),
            {},
            class_type,
            range->AsField(),
            false);
        auto end_function = builder.FindFunction(
            class_type->GetFunctions("end"),
            {},
            class_type,
            range->AsField(),
            false);

        Assert(begin_function.has_value(), "class is missing function 'begin'");
        Assert(end_function.has_value(), "class is missing function 'end'");

        begin = builder.CreateCall(begin_function->Type, begin_function->Callee, {}, range);
        end = builder.CreateCall(end_function->Type, end_function->Callee, {}, range);

        break;
    }
    default:
        Error("iterating over value of type {} not implemented", type);
    }

    ValuePtr iterator;
    {
        const auto pointer = builder.CreateAlloca(begin->GetType());
        builder.CreateStore(pointer, begin);
        iterator = Value::CreateL(begin->GetType(), pointer, true);
    }

    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    const auto loop_block = builder.CreateBlock("loop", parent);
    const auto end_block = builder.CreateBlock("end", parent);

    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);

    if (m_Name.front() != '_')
    {
        const auto operator_ = builder.FindOperator("*", iterator->AsField(), false);
        Assert(operator_ != nullptr, "operator '*' not implemented for {}", iterator->AsField());

        auto value = (*operator_)(builder, iterator);

        if (m_Reference)
        {
            Assert(value->IsReferenceable(), "value is not referenceable");
            Assert(!m_Mutable || value->IsMutable(), "reference mutability violation");

            value = Value::CreateL(value->GetType(), value->GetPointer(), m_Mutable);
        }
        else if (m_Mutable)
        {
            auto pointer = builder.CreateAlloca(value->GetType());
            builder.CreateStore(pointer, value);

            value = Value::CreateL(value->GetType(), pointer, true);
        }
        else
        {
            value = Value::CreateR(value->GetType(), value->Load(builder));
        }

        builder.SetValue(m_Name, std::move(value));
    }

    {
        const auto operator_ = builder.FindOperator("!=", iterator->AsField(), end->AsField());
        Assert(
            operator_ != nullptr,
            "operator '!=' not implemented for {} and {}",
            iterator->AsField(),
            end->AsField());

        const auto condition = (*operator_)(builder, iterator, end);
        builder.CreateBranch(condition, loop_block, end_block);
    }

    builder.SetInsertPoint(loop_block);

    m_Content->Gen(builder);

    {
        const auto operator_ = builder.FindOperator("++", iterator->AsField(), false);
        Assert(operator_ != nullptr, "operator '++' not implemented for {}", iterator->AsField());

        (void) (*operator_)(builder, iterator);
    }

    if (builder.NoTerminator())
        builder.CreateBranch(head_block);

    builder.PopFrame();

    builder.SetInsertPoint(end_block);
}

llove::StatementPtr llove::ForEachStatement::Reflect(Context &types) const
{
    ExpressionPtr range;
    StatementPtr content;

    if (m_Range)
        m_Range->Reflect(types, range);
    if (m_Content)
        m_Content->Reflect(types, content);

    return std::make_unique<ForEachStatement>(m_Mutable, m_Reference, m_Name, std::move(range), std::move(content));
}

std::ostream &llove::ForEachStatement::Print(std::ostream &stream) const
{
    return stream
           << "foreach ("
           << (m_Mutable ? "mut " : "")
           << (m_Reference ? "&" : "")
           << m_Name
           << " : "
           << m_Range
           << ") "
           << m_Content;
}
