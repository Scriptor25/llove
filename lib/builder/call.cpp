#include <llove/builder.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::CreateCall(
    const FunctionReference &reference,
    std::vector<ValuePtr> arguments,
    ValuePtr self)
{
    const auto &function_type = reference.Type;
    const auto &function_self = function_type->GetSelf();
    const auto &function_result = function_type->GetResult();

    Assert(!self == !function_self, "illegal function call, function self does not match self");

    std::vector<llvm::Value *> argument_values;

    if (self)
        argument_values.push_back(function_self->GenCast(*this, std::move(self)));

    unsigned i;
    for (i = 0; i < function_type->GetParameterCount(); ++i)
    {
        auto &parameter = function_type->GetParameter(i);
        auto &argument = arguments[i];

        argument_values.push_back(parameter.GenCast(*this, std::move(argument)));
    }

    if (function_type->HasVariadic())
    {
        if (const auto count = arguments.size() - i; count == 1 && arguments[i]->GetType()->IsVariadic())
            argument_values.push_back(arguments[i++]->Load(*this));
        else
        {
            std::vector<llvm::Value *> values;
            for (; i < arguments.size(); ++i)
            {
                const auto &argument = arguments[i];
                const auto argument_type = argument->GetType();

                std::vector<llvm::Constant *> typeinfo_values;
                Assert(argument_type->TypeInfo(*this, typeinfo_values), "invalid typeinfo for {}", argument_type);

                const auto typeinfo_type = llvm::ConstantStruct::getTypeForElements(
                    m_LLVMContext,
                    typeinfo_values,
                    true);
                const auto typeinfo_value = llvm::ConstantStruct::get(typeinfo_type, typeinfo_values);
                const auto typeinfo_pointer = CreateAlloca(typeinfo_type);
                CreateStore(typeinfo_value, typeinfo_pointer);

                const auto value = argument->Load(*this);
                const auto bits = argument->GetType()->SizeBits(*this);
                const auto bytes = (bits >> 3) + ((bits & 7) != 0);

                values.push_back(GetI32(bytes));
                values.push_back(typeinfo_pointer);
                values.push_back(value);
            }

            const auto variadic_type = GetVariadicType();
            const auto count_type = variadic_type->getElementType(0);
            const auto count_value = llvm::ConstantInt::get(count_type, count);

            std::vector<llvm::Type *> types;
            for (const auto value : values)
                types.push_back(value->getType());

            const auto data_type = llvm::StructType::get(m_LLVMContext, types, true);
            const auto data_pointer = CreateAlloca(data_type);

            llvm::Value *data_value = llvm::Constant::getNullValue(data_type);
            for (unsigned j = 0; j < values.size(); ++j)
                data_value = CreateInsertValue(data_value, values[j], j);
            CreateStore(data_value, data_pointer);

            llvm::Value *variadic_value = llvm::Constant::getNullValue(variadic_type);
            variadic_value = m_LLVMBuilder.CreateInsertValue(variadic_value, count_value, 0);
            variadic_value = m_LLVMBuilder.CreateInsertValue(variadic_value, data_pointer, 1);

            argument_values.push_back(variadic_value);
        }
    }

    const auto result_value = m_LLVMBuilder.CreateCall(
        function_type->GenFunction(*this),
        reference.Callee,
        argument_values);

    if (function_result.IsReference())
        return Value::CreateL(function_result.GetType(), result_value, function_result.IsMutable());

    return Value::CreateR(function_result.GetType(), result_value);
}

llove::ValuePtr llove::Builder::CreateCall(const ValuePtr &callee)
{
    const auto function_type = As<FunctionType>(callee->GetType());
    auto &function_result = function_type->GetResult();

    std::vector<llvm::Value *> arguments;
    if (function_type->HasVariadic())
        arguments.push_back(llvm::Constant::getNullValue(GetVariadicType()));

    const auto result_value = m_LLVMBuilder.CreateCall(
        function_type->GenFunction(*this),
        callee->Load(*this),
        arguments);

    if (function_result.IsReference())
        return Value::CreateL(function_result.GetType(), result_value, function_result.IsMutable());

    return Value::CreateR(function_result.GetType(), result_value);
}
