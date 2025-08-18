#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ForEachStatement::ForEachStatement(
    Location loc,
    const bool mutable_,
    const bool reference,
    std::string name,
    ExpressionPtr range,
    StatementPtr content)
    : Statement(std::move(loc)),
      m_Mutable(mutable_),
      m_Reference(reference),
      m_Name(std::move(name)),
      m_Range(std::move(range)),
      m_Content(std::move(content))
{
}

void llove::ForEachStatement::Gen(Builder &builder) const try
{
    builder.EmitLoc(m_Loc);
    builder.PushFrame(m_Loc);

    const auto range = m_Range->GenVal(builder, nullptr);
    const auto type = range->GetType();

    ValuePtr begin;
    ValuePtr end;

    builder.EmitLoc(m_Loc);

    switch (type->GetId())
    {
    case TypeId_Array:
    {
        const auto array_type = As<ArrayType>(type);
        const auto base_type = array_type->GetBase();
        auto iterator_type = builder.GetContext().GetPointer(base_type, range->IsMutable());

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

        auto &begin_fld = struct_type->GetField(begin_index);
        auto &end_fld = struct_type->GetField(end_index);

        if (range->IsReferenceable())
        {
            auto begin_pointer = builder.CreateStructGEP(struct_type, range->GetPointer(), begin_index);
            if (begin_fld.Reference)
            {
                begin_pointer = builder.CreateLoad(
                    begin_pointer,
                    builder.GetContext().GetPointer(begin_fld.Type, begin_fld.Mutable));
                begin = Value::CreateL(begin_fld.Type, begin_pointer, begin_fld.Mutable);
            }
            else
            {
                begin = Value::CreateL(begin_fld.Type, begin_pointer, range->IsMutable() && begin_fld.Mutable);
            }

            auto end_pointer = builder.CreateStructGEP(struct_type, range->GetPointer(), end_index);
            if (end_fld.Reference)
            {
                end_pointer = builder.CreateLoad(
                    end_pointer,
                    builder.GetContext().GetPointer(end_fld.Type, end_fld.Mutable));
                end = Value::CreateL(end_fld.Type, end_pointer, end_fld.Mutable);
            }
            else
            {
                end = Value::CreateL(end_fld.Type, end_pointer, range->IsMutable() && end_fld.Mutable);
            }
        }
        else
        {
            auto begin_value = builder.CreateExtractValue(range, begin_index);
            if (begin_fld.Reference)
            {
                begin_value = builder.CreateLoad(
                    begin_value,
                    builder.GetContext().GetPointer(begin_fld.Type, begin_fld.Mutable));
                begin = Value::CreateL(begin_fld.Type, begin_value, begin_fld.Mutable);
            }
            else
            {
                begin = Value::CreateR(begin_fld.Type, begin_value);
            }

            auto end_value = builder.CreateExtractValue(range, end_index);
            if (end_fld.Reference)
            {
                end_value = builder.CreateLoad(
                    end_value,
                    builder.GetContext().GetPointer(end_fld.Type, end_fld.Mutable));
                end = Value::CreateL(end_fld.Type, end_value, end_fld.Mutable);
            }
            else
            {
                end = Value::CreateR(end_fld.Type, end_value);
            }
        }

        if (begin_fld.Type->IsFunction())
        {
            const auto fn_type = As<FunctionType>(begin_fld.Type);
            Assert(!fn_type->IsVarArg(), "illegal vararg");
            Assert(fn_type->GetParameterCount() == 0, "illegal parameter count");

            begin = builder.CreateCall(begin);
        }

        if (end_fld.Type->IsFunction())
        {
            const auto fn_type = As<FunctionType>(end_fld.Type);
            Assert(!fn_type->IsVarArg(), "illegal vararg");
            Assert(fn_type->GetParameterCount() == 0, "illegal parameter count");

            end = builder.CreateCall(end);
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

        begin = builder.CreateCall(*begin_function, {}, range);
        end = builder.CreateCall(*end_function, {}, range);

        break;
    }
    default:
        Error("iterating over value of type {} not implemented", type);
    }

    builder.EmitLoc(m_Loc);

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
        ValuePtr storage;

        if (const auto iterator_type = iterator->GetType(); iterator_type->IsInteger() || iterator_type->IsFloat())
        {
            storage = Value::CreateR(iterator_type, iterator->Load(builder));
        }
        else
        {
            const auto operator_ = builder.FindOperator("*", iterator->AsField(), false);
            Assert(operator_ != nullptr, "operator '*' not implemented for {}", iterator->AsField());

            storage = (*operator_)(builder, iterator);
        }

        if (m_Reference)
        {
            Assert(storage->IsReferenceable(), "value is not referenceable");
            Assert(!m_Mutable || storage->IsMutable(), "reference mutability violation");

            storage = Value::CreateL(storage->GetType(), storage->GetPointer(), m_Mutable);
        }
        else if (m_Mutable)
        {
            auto pointer = builder.CreateAlloca(storage->GetType());
            builder.CreateStore(pointer, storage);

            storage = Value::CreateL(storage->GetType(), pointer, true);
        }
        else
        {
            storage = Value::CreateR(storage->GetType(), storage->Load(builder));
        }

        builder.GetDebug().CreateVariable(builder, m_Name, storage);
        builder.SetValue(m_Name, std::move(storage));
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

    builder.EmitLoc(m_Loc);
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
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::ForEachStatement::Reflect(Context &context) const try
{
    ExpressionPtr range;
    StatementPtr content;

    if (m_Range)
        m_Range->Reflect(context, range);
    if (m_Content)
        m_Content->Reflect(context, content);

    return std::make_unique<ForEachStatement>(
        m_Loc,
        m_Mutable,
        m_Reference,
        m_Name,
        std::move(range),
        std::move(content));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
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
