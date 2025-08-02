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
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    template<>
    class Operator<2>
    {
    public:
        using Ptr = std::unique_ptr<Operator>;

        virtual ~Operator() = default;
        virtual ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    template<>
    class BIOperator<1> final : public Operator<1>
    {
    public:
        using CalleeType = std::function<ValuePtr(Builder &builder, ValuePtr operand, bool suffix)>;

        explicit BIOperator(CalleeType callee, bool suffix);

        ValuePtr operator()(Builder &builder, ValuePtr operand) const override;
        std::ostream &Print(std::ostream &stream) const override;

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
        std::ostream &Print(std::ostream &stream) const override;

    private:
        CalleeType m_Callee;
    };

    template<>
    class UDOperator<1> final : public Operator<1>
    {
    public:
        explicit UDOperator(const FunctionReference &reference);

        ValuePtr operator()(Builder &builder, ValuePtr operand) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        const FunctionReference &m_Reference;
    };

    template<>
    class UDOperator<2> final : public Operator<2>
    {
    public:
        explicit UDOperator(const FunctionReference &reference);

        ValuePtr operator()(Builder &builder, ValuePtr left, ValuePtr right) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        const FunctionReference &m_Reference;
    };

    extern const std::map<std::string_view, BIOperator<1>::CalleeType> BIUnOperatorCallees;
    extern const std::map<std::string_view, BIOperator<2>::CalleeType> BIBiOperatorCallees;
}

template<typename T> requires std::is_base_of_v<llove::Operator<1>, T>
struct std::formatter<std::unique_ptr<T>> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const std::unique_ptr<T> &operator_, FormatContext &ctx) const
    {
        std::stringstream stream;
        operator_->Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};

template<typename T> requires std::is_base_of_v<llove::Operator<2>, T>
struct std::formatter<std::unique_ptr<T>> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const std::unique_ptr<T> &operator_, FormatContext &ctx) const
    {
        std::stringstream stream;
        operator_->Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};
