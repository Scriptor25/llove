#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::VariadicExpression::VariadicExpression(Location loc, ExpressionPtr list, TypePtr type)
    : Expression(std::move(loc)),
      m_List(std::move(list)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::VariadicExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    // !list   => <empty:i1>
    // *list   => struct { type: <typeinfo:ptr>, data: <pointer:ptr> }
    // ++list  => variadic { count: <count:i32> - 1, data: <pointer:ptr> + <offset from typeinfo> }

    const auto list = m_List->GenVal(builder, builder.GetContext().GetVariadic());
    const auto type = m_Type->GenIR(builder);

    const auto list_type = list->GetType();
    Assert(list_type->IsVariadic(), "list operand must be variadic");

    const auto list_pointer = list->GetPointer();
    const auto count_pointer = builder.CreateStructGEP(list_type->GenIR(builder), list_pointer, 0);
    const auto data_pointer = builder.CreateStructGEP(list_type->GenIR(builder), list_pointer, 1);

    const auto count = builder.CreateLoad(builder.GetIntegerType(32), count_pointer);
    const auto data = builder.CreateLoad(builder.GetPointerType(), data_pointer);

    const auto get_block = builder.CreateBlock("get", builder.GetParent());
    const auto empty_block = builder.CreateBlock("empty", builder.GetParent());
    const auto end_block = builder.CreateBlock("end", builder.GetParent());

    const auto condition = builder.CreateIsNotNull(count);
    builder.CreateBranch(condition, get_block, empty_block);

    builder.SetInsertPoint(get_block);
    const auto get_value = builder.CreateLoad(type, data);
    llvm::Value *aggregate = llvm::Constant::getNullValue(list_type->GenIR(builder));
    aggregate = builder.CreateInsertValue(
        aggregate,
        builder.CreateSub(count, llvm::ConstantInt::get(count->getType(), 1)),
        0);
    aggregate = builder.CreateInsertValue(
        aggregate,
        builder.CreateGEP(type, data, 1),
        1);
    builder.CreateStore(aggregate, list_pointer);
    builder.CreateBranch(end_block);

    builder.SetInsertPoint(empty_block);
    const auto empty_value = llvm::Constant::getNullValue(type);
    builder.CreateBranch(end_block);

    builder.SetInsertPoint(end_block);

    const auto value = builder.CreatePHI(
        type,
        {
            { get_block, get_value },
            { empty_block, empty_value },
        });

    return Value::CreateR(m_Type, value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::VariadicExpression::Reflect(Context &context) const try
{
    ExpressionPtr list;
    TypePtr type;

    if (m_List)
        m_List->Reflect(context, list);
    if (m_Type)
        Type::Reflect(context, m_Type, type);

    return std::make_unique<VariadicExpression>(m_Loc, std::move(list), std::move(type));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::VariadicExpression::Print(std::ostream &stream) const
{
    return stream << m_List << '{' << m_Type << '}';
}
