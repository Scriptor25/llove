#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <llove/class.hpp>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/parameter.hpp>
#include <llove/type.hpp>

namespace llove
{
    class Global
    {
    public:
        virtual ~Global() = default;
        virtual void Gen(Builder &builder) const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    class ClassGlobal final : public Global
    {
    public:
        explicit ClassGlobal(ClassType::Ptr type);
        explicit ClassGlobal(
            ClassType::Ptr type,
            std::vector<ClassFieldReference> fields,
            std::vector<ClassFunction> functions);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ClassType::Ptr m_Type;
        bool m_Opaque;
        std::vector<ClassFieldReference> m_Fields;
        std::vector<ClassFunction> m_Functions;
    };

    class ClassDefinitionGlobal final : public Global
    {
    public:
        explicit ClassDefinitionGlobal(
            ClassType::Ptr class_type,
            bool mutable_,
            std::string name,
            std::vector<Parameter> parameters,
            bool vararg,
            Field result,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ClassType::Ptr m_ClassType;
        bool m_Mutable;
        std::string m_Name;
        std::vector<Parameter> m_Parameters;
        bool m_VarArg;
        Field m_Result;
        StatementPtr m_Content;
    };

    class DefinitionGlobal final : public Global
    {
    public:
        explicit DefinitionGlobal(
            bool interface,
            std::string name,
            std::vector<Parameter> parameters,
            bool vararg,
            Field result,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        bool m_Interface;
        std::string m_Name;
        std::vector<Parameter> m_Parameters;
        bool m_VarArg;
        Field m_Result;
        StatementPtr m_Content;
    };

    class Statement
    {
    public:
        virtual ~Statement() = default;
        virtual void Gen(Builder &builder) const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    class ForStatement final : public Statement
    {
    public:
        explicit ForStatement(StatementPtr prefix, StatementPtr suffix, ExpressionPtr condition, StatementPtr content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        StatementPtr m_Prefix;
        StatementPtr m_Suffix;
        ExpressionPtr m_Condition;
        StatementPtr m_Content;
    };

    class ForEachStatement final : public Statement
    {
    public:
        explicit ForEachStatement(
            bool mutable_,
            bool reference,
            std::string name,
            ExpressionPtr range,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        bool m_Mutable;
        bool m_Reference;
        std::string m_Name;
        ExpressionPtr m_Range;
        StatementPtr m_Content;
    };

    class IfStatement final : public Statement
    {
    public:
        explicit IfStatement(ExpressionPtr condition, StatementPtr then, StatementPtr else_);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Condition;
        StatementPtr m_Then;
        StatementPtr m_Else;
    };

    class LetStatement final : public Statement
    {
    public:
        explicit LetStatement(Field info, std::string name, ExpressionPtr value, std::vector<ExpressionPtr> arguments);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        Field m_Info;
        std::string m_Name;
        ExpressionPtr m_Value;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class ScopeStatement final : public Statement
    {
    public:
        static StatementPtr Wrap(StatementPtr ptr);

        explicit ScopeStatement(std::vector<StatementPtr> content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<StatementPtr> m_Content;
    };

    class YieldStatement final : public Statement
    {
    public:
        explicit YieldStatement(ExpressionPtr value);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
    };

    struct CalleeInfo
    {
        std::vector<FunctionReference> Candidates;
        ValuePtr Self;
    };

    class Expression : public Statement
    {
    public:
        void Gen(Builder &builder) const override;
        virtual ValuePtr GenVal(Builder &builder, TypePtr expect) const = 0;
        virtual CalleeInfo GenCallee(Builder &builder) const;
    };

    class ArrayExpression final : public Expression
    {
    public:
        explicit ArrayExpression(std::vector<ExpressionPtr> values, ArrayType::Ptr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<ExpressionPtr> m_Values;
        ArrayType::Ptr m_Type;
    };

    class BinaryExpression final : public Expression
    {
    public:
        explicit BinaryExpression(std::string operator_, ExpressionPtr left, ExpressionPtr right);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Left;
        ExpressionPtr m_Right;
    };

    class CallExpression final : public Expression
    {
    public:
        explicit CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> arguments);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Callee;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class IntExpression final : public Expression
    {
    public:
        explicit IntExpression(uint64_t value, IntegerType::Ptr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        uint64_t m_Value;
        IntegerType::Ptr m_Type;
    };

    class MemberExpression final : public Expression
    {
    public:
        explicit MemberExpression(ExpressionPtr value, std::string member);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        CalleeInfo GenCallee(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
        std::string m_Member;
    };

    class NullExpression final : public Expression
    {
    public:
        explicit NullExpression(TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Type;
    };

    class RangeExpression final : public Expression
    {
    public:
        explicit RangeExpression(ExpressionPtr begin, ExpressionPtr end);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Begin;
        ExpressionPtr m_End;
    };

    class StringExpression final : public Expression
    {
    public:
        explicit StringExpression(std::string value);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Value;
    };

    class StructExpression final : public Expression
    {
    public:
        explicit StructExpression(std::map<std::string, ExpressionPtr> values, StructType::Ptr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::map<std::string, ExpressionPtr> m_Values;
        StructType::Ptr m_Type;
    };

    class SubscriptExpression final : public Expression
    {
    public:
        explicit SubscriptExpression(ExpressionPtr value, ExpressionPtr index);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
        ExpressionPtr m_Index;
    };

    class SymbolExpression final : public Expression
    {
    public:
        explicit SymbolExpression(std::string name);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        CalleeInfo GenCallee(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
    };

    class UnaryExpression final : public Expression
    {
    public:
        explicit UnaryExpression(std::string operator_, ExpressionPtr operand, bool suffix);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Operand;
        bool m_Suffix;
    };

    extern unsigned PrintDepth;
}
