#pragma once

#include <functional>
#include <map>
#include <llove/forward.hpp>
#include <llove/type.hpp>

namespace llove
{
    template<unsigned C>
    class Operator;

    template<unsigned C>
    class BIOperator;

    template<unsigned C>
    class UDOperator;

    template<>
    class Operator<1>
    {
    public:
        using Ptr = std::unique_ptr<Operator>;

        virtual ~Operator() = default;
        virtual ValuePtr operator()(Builder &builder, ValuePtr operand) const = 0;
    };

    template<>
    class Operator<2>
    {
    public:
        using Ptr = std::unique_ptr<Operator>;

        virtual ~Operator() = default;
        virtual ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const = 0;
    };

    template<>
    class BIOperator<1> final : public Operator<1>
    {
    public:
        using CalleeType = std::function<ValuePtr(Builder &builder, ValuePtr operand, bool suffix)>;

        explicit BIOperator(CalleeType callee, bool suffix);

        ValuePtr operator()(Builder &builder, ValuePtr operand) const override;

    private:
        CalleeType m_Callee;
        bool m_Suffix;
    };

    template<>
    class BIOperator<2> final : public Operator<2>
    {
    public:
        using CalleeType = std::function<ValuePtr(Builder &builder, ValuePtr left, ValuePtr right)>;

        explicit BIOperator(CalleeType callee);

        ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const override;

    private:
        CalleeType m_Callee;
    };

    template<>
    class UDOperator<1> final : public Operator<1>
    {
    public:
        explicit UDOperator(FunctionType::Ptr type, llvm::Value *callee);

        ValuePtr operator()(Builder &builder, ValuePtr operand) const override;

    private:
        FunctionType::Ptr m_Type;
        llvm::Value *m_Callee;
    };

    template<>
    class UDOperator<2> final : public Operator<2>
    {
    public:
        explicit UDOperator(FunctionType::Ptr type, llvm::Value *callee);

        ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const override;

    private:
        FunctionType::Ptr m_Type;
        llvm::Value *m_Callee;
    };

    extern const std::map<std::string_view, BIOperator<1>::CalleeType> BIUnOperatorCallees;
    extern const std::map<std::string_view, BIOperator<2>::CalleeType> BIBiOperatorCallees;
}
