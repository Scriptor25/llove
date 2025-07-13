#pragma once

#include <functional>
#include <map>
#include <llove/forward.hpp>
#include <llove/type.hpp>
#include <llvm/IR/Function.h>

namespace llove
{
    class Operator
    {
    public:
        using Ptr = std::unique_ptr<Operator>;

        virtual ~Operator() = default;

        virtual ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const = 0;
    };

    class BuiltinOperator final : public Operator
    {
    public:
        using CalleeType = std::function<ValuePtr(Builder &builder, ValuePtr left, ValuePtr right)>;

        explicit BuiltinOperator(CalleeType callee);

        ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const override;

    private:
        CalleeType m_Callee;
    };

    class UserDefinedOperator final : public Operator
    {
    public:
        explicit UserDefinedOperator(FunctionType::Ptr type, llvm::Value *callee);

        ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const override;

    private:
        FunctionType::Ptr m_Type;
        llvm::Value *m_Callee;
    };

    extern std::map<std::string, BuiltinOperator::CalleeType> BuiltinOperatorCallees;
}
