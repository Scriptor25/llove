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

    TypePtr iterator_type;
    llvm::Value *begin;
    llvm::Value *end;

    switch (type->GetId())
    {
    case TypeId_Array:
    {
        const auto array_type = As<ArrayType>(type);
        iterator_type = builder.GetTypes().GetPointer(array_type->GetBase(), range->IsMutable());

        if (range->IsReferenceable())
        {
            begin = range->GetPointer();
        }
        else
        {
            begin = builder.CreateAlloca(type);
            builder.CreateStore(begin, range);
        }

        end = builder.CreatePointerOffset(array_type->GetBase()->Gen(builder), begin, array_type->GetSize());
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
            auto pointer = builder.CreateStructGEP(struct_type, range->GetPointer(), begin_index);
            if (begin_reference)
                pointer = builder.CreateLoad(pointer, builder.GetTypes().GetPointer(begin_type, begin_mutable));
            begin = builder.CreateLoad(pointer, begin_type);
        }
        else
        {
            begin = builder.CreateExtractValue(range, begin_index);
            if (begin_reference)
                begin = builder.CreateLoad(begin, builder.GetTypes().GetPointer(begin_type, begin_mutable));
        }
        if (begin_type->GetId() == TypeId_Function)
        {
            const auto fn_type = As<FunctionType>(begin_type);
            Assert(!fn_type->IsVarArg(), "invalid vararg");
            Assert(fn_type->GetParameterCount() == 0, "invalid parameter count");

            begin = builder.CreateCall(fn_type, begin, {});
            if (fn_type->GetResult().Reference)
                begin = builder.CreateLoad(begin, fn_type->GetResult().Type);

            iterator_type = fn_type->GetResult().Type;
        }
        else
        {
            iterator_type = begin_type;
        }

        if (range->IsReferenceable())
        {
            auto pointer = builder.CreateStructGEP(struct_type, range->GetPointer(), end_index);
            if (end_reference)
                pointer = builder.CreateLoad(pointer, builder.GetTypes().GetPointer(end_type, end_mutable));
            end = builder.CreateLoad(pointer, end_type);
        }
        else
        {
            end = builder.CreateExtractValue(range, end_index);
            if (end_reference)
                end = builder.CreateLoad(end, builder.GetTypes().GetPointer(end_type, end_mutable));
        }
        if (end_type->GetId() == TypeId_Function)
        {
            const auto fn_type = As<FunctionType>(end_type);
            Assert(!fn_type->IsVarArg(), "invalid vararg");
            Assert(fn_type->GetParameterCount() == 0, "invalid parameter count");

            end = builder.CreateCall(fn_type, end, {});
            if (fn_type->GetResult().Reference)
                end = builder.CreateLoad(end, fn_type->GetResult().Type);

            Assert(fn_type->GetResult().Type == iterator_type, "iterator type mismatch");
        }
        else
        {
            Assert(end_type == iterator_type, "iterator type mismatch");
        }

        break;
    }
    case TypeId_Range:
    {
        const auto range_type = As<RangeType>(type);
        iterator_type = range_type->GetEntry();

        const auto value = range->Load(builder);
        begin = builder.CreateExtractValue(value, 0);
        end = builder.CreateExtractValue(value, 1);
        break;
    }
    case TypeId_Class:
        Error("not implemented");
    default:
        Error("iterating over value of type {} not implemented", type);
    }

    const auto iterator_pointer = builder.CreateAlloca(iterator_type);
    builder.CreateStore(iterator_pointer, begin);

    const auto parent = builder.GetParent();
    const auto head_block = builder.CreateBlock("head", parent);
    const auto loop_block = builder.CreateBlock("loop", parent);
    const auto end_block = builder.CreateBlock("end", parent);

    builder.CreateBranch(head_block);

    builder.SetInsertPoint(head_block);
    const auto pos = builder.CreateLoad(iterator_pointer, iterator_type);
    const auto condition = builder.CreateCompareNE(iterator_type, pos, end);
    if (m_Name.front() != '_')
    {
        const auto operator_ = builder.FindOperator("*", { false, true, iterator_type }, false);
        Assert(operator_ != nullptr, "not implemented");

        auto value = (*operator_)(builder, Value::CreateL(iterator_type, iterator_pointer, false));

        if (m_Reference)
        {
            Assert(value->IsReferenceable(), "value is not referenceable");
            Assert(!m_Mutable || value->IsMutable(), "reference mutability violation");
        }

        builder.SetValue(m_Name, std::move(value));
    }
    builder.CreateBranch(condition, loop_block, end_block);

    builder.SetInsertPoint(loop_block);
    m_Content->Gen(builder);
    const auto next_pos = builder.CreateIncrement(iterator_type, pos);
    builder.CreateStore(iterator_pointer, next_pos);
    if (builder.NoTerminator())
        builder.CreateBranch(head_block);

    builder.PopFrame();

    builder.SetInsertPoint(end_block);
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
