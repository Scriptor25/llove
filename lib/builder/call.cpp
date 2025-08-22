#include <llove/builder.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::CreateCall(
    const FunctionReference &function,
    std::vector<ValuePtr> arguments,
    ValuePtr self)
{
    auto &function_type = function.Type;
    auto &function_self = function_type->GetSelf();
    auto &function_result = function_type->GetResult();

    Assert(!self == !function_self, "illegal function call, function self does not match self");

    std::vector<llvm::Value *> argument_values;

    if (self)
    {
        argument_values.emplace_back(function_self->GenCast(*this, std::move(self)));
    }

    unsigned i;
    for (i = 0; i < function_type->GetParameterCount(); ++i)
    {
        auto &parameter = function_type->GetParameter(i);
        auto &argument = arguments.at(i);

        argument_values.emplace_back(parameter.GenCast(*this, std::move(argument)));
    }

    if (function_type->HasVariadic())
    {
        if (const auto count = arguments.size() - i; count == 1 && arguments.at(i)->GetType()->IsVariadic())
        {
            argument_values.emplace_back(arguments.at(i++)->Load(*this));
        }
        else
        {
            const auto type = GetVariadicType();

            std::vector<llvm::Type *> elements;
            for (auto j = i; j < arguments.size(); ++j)
            {
                const auto argument_type = arguments.at(j)->GetType();
                elements.emplace_back(argument_type->GenIR(*this));
            }

            const auto count_type = type->getElementType(0);
            const auto count_value = llvm::ConstantInt::get(count_type, count, false);

            const auto elements_type = llvm::StructType::get(m_LLVMContext, elements, true);
            const auto elements_pointer = CreateAlloca(elements_type);

            for (auto j = 0; i < arguments.size(); ++i, ++j)
            {
                const auto value = arguments.at(i)->Load(*this);
                const auto pointer = m_LLVMBuilder.CreateStructGEP(elements_type, elements_pointer, j);
                m_LLVMBuilder.CreateStore(value, pointer);
            }

            llvm::Value *aggregate = llvm::Constant::getNullValue(type);
            aggregate = m_LLVMBuilder.CreateInsertValue(aggregate, count_value, 0);
            aggregate = m_LLVMBuilder.CreateInsertValue(aggregate, elements_pointer, 1);

            argument_values.emplace_back(aggregate);
        }
    }

    const auto result_value = m_LLVMBuilder.CreateCall(
        function_type->GenFunction(*this),
        function.Callee,
        argument_values);

    if (function_result.Reference)
        return Value::CreateL(function_result.Type, result_value, function_result.Mutable);

    return Value::CreateR(function_result.Type, result_value);
}

llove::ValuePtr llove::Builder::CreateCall(const ValuePtr &callee)
{
    const auto function_type = As<FunctionType>(callee->GetType());
    auto &function_result = function_type->GetResult();

    std::vector<llvm::Value *> arguments;
    if (function_type->HasVariadic())
    {
        arguments.emplace_back(llvm::Constant::getNullValue(GetVariadicType()));
    }

    const auto result_value = m_LLVMBuilder.CreateCall(
        function_type->GenFunction(*this),
        callee->Load(*this),
        arguments);

    if (function_result.Reference)
        return Value::CreateL(function_result.Type, result_value, function_result.Mutable);

    return Value::CreateR(function_result.Type, result_value);
}
