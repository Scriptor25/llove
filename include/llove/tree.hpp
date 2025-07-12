#pragma once

#include <map>
#include <memory>
#include <llove/class.hpp>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    class Global
    {
    public:
        virtual ~Global() = default;
        virtual void Gen(Builder &builder) const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
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

    class ClassDefinitionGlobal final : public Global
    {
    public:
        explicit ClassDefinitionGlobal(
            std::string class_name,
            bool mutable_,
            std::string name,
            std::vector<Parameter> parameters,
            bool vararg,
            Field result,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_ClassName;
        bool m_Mutable;
        std::string m_Name;
        std::vector<Parameter> m_Parameters;
        bool m_VarArg;
        Field m_Result;
        StatementPtr m_Content;
    };

    class ClassGlobal final : public Global
    {
    public:
        explicit ClassGlobal(std::string name);
        explicit ClassGlobal(std::string name, std::vector<ClassField> fields, std::vector<ClassFunction> functions);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
        bool m_Opaque;
        std::vector<ClassField> m_Fields;
        std::vector<ClassFunction> m_Functions;
    };

    class Statement
    {
    public:
        virtual ~Statement() = default;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    class ScopeStatement final : public Statement
    {
    public:
        explicit ScopeStatement(std::vector<StatementPtr> content);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<StatementPtr> m_Content;
    };

    class LetStatement final : public Statement
    {
    public:
        explicit LetStatement(Field info, std::string name, ExpressionPtr value);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        Field m_Info;
        std::string m_Name;
        ExpressionPtr m_Value;
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

        std::ostream &Print(std::ostream &stream) const override;

    private:
        bool m_Mutable;
        bool m_Reference;
        std::string m_Name;
        ExpressionPtr m_Range;
        StatementPtr m_Content;
    };

    class YieldStatement final : public Statement
    {
    public:
        explicit YieldStatement(ExpressionPtr value);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
    };

    class Expression : public Statement
    {
    };

    class NullExpression final : public Expression
    {
    public:
        explicit NullExpression(TypePtr type);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Type;
    };

    class IntExpression final : public Expression
    {
    public:
        explicit IntExpression(uint64_t value, TypePtr type);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        uint64_t m_Value;
        TypePtr m_Type;
    };

    class StringExpression final : public Expression
    {
    public:
        explicit StringExpression(std::string value);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Value;
    };

    class RangeExpression final : public Expression
    {
    public:
        explicit RangeExpression(ExpressionPtr begin, ExpressionPtr end);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Begin;
        ExpressionPtr m_End;
    };

    class ArrayExpression final : public Expression
    {
    public:
        explicit ArrayExpression(std::vector<ExpressionPtr> values, TypePtr type);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<ExpressionPtr> m_Values;
        TypePtr m_Type;
    };

    class StructExpression final : public Expression
    {
    public:
        explicit StructExpression(std::map<std::string, ExpressionPtr> values, TypePtr type);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::map<std::string, ExpressionPtr> m_Values;
        TypePtr m_Type;
    };

    class SymbolExpression final : public Expression
    {
    public:
        explicit SymbolExpression(std::string name);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
    };

    class BinaryExpression final : public Expression
    {
    public:
        explicit BinaryExpression(std::string operator_, ExpressionPtr left, ExpressionPtr right);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Left;
        ExpressionPtr m_Right;
    };

    class UnaryExpression final : public Expression
    {
    public:
        explicit UnaryExpression(std::string operator_, ExpressionPtr operand, bool suffix);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Operand;
        bool m_Suffix;
    };

    class CallExpression final : public Expression
    {
    public:
        explicit CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> arguments);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Callee;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class MemberExpression final : public Expression
    {
    public:
        explicit MemberExpression(ExpressionPtr value, std::string member);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
        std::string m_Member;
    };

    class SubscriptExpression final : public Expression
    {
    public:
        explicit SubscriptExpression(ExpressionPtr value, ExpressionPtr index);

        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
        ExpressionPtr m_Index;
    };
}
