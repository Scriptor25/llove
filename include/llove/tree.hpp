#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <llove/class.hpp>
#include <llove/error.hpp>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/location.hpp>
#include <llove/parameter.hpp>
#include <llove/type.hpp>

namespace llove
{
    class Global
    {
    public:
        explicit Global(Location loc);

        [[nodiscard]] const Location &Loc() const;

        virtual ~Global() = default;
        virtual void Gen(Builder &builder) const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;

    protected:
        Location m_Loc;
    };

    class ClassGlobal final : public Global
    {
    public:
        explicit ClassGlobal(Location loc, ClassType::Ptr type);
        explicit ClassGlobal(
            Location loc,
            ClassType::Ptr type,
            std::vector<ClassField> fields,
            std::vector<ClassFunction> functions);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ClassType::Ptr m_Type;
        bool m_Opaque;
        std::vector<ClassField> m_Fields;
        std::vector<ClassFunction> m_Functions;
    };

    class ClassDefinitionGlobal final : public Global
    {
    public:
        explicit ClassDefinitionGlobal(
            Location loc,
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
            Location loc,
            bool interface,
            bool implicit,
            std::string name,
            std::vector<Parameter> parameters,
            bool vararg,
            Field result,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        bool m_Interface;
        bool m_Implicit;
        std::string m_Name;
        std::vector<Parameter> m_Parameters;
        bool m_VarArg;
        Field m_Result;
        StatementPtr m_Content;
    };

    class Statement
    {
    public:
        explicit Statement(Location loc);

        [[nodiscard]] const Location &Loc() const;

        virtual ~Statement() = default;
        virtual void Gen(Builder &builder) const = 0;
        virtual StatementPtr Reflect(Builder &builder) const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;

        template<typename T> requires std::is_base_of_v<Statement, T>
        void Reflect(Builder &builder, std::unique_ptr<T> &ref) const
        {
            auto ptr = Reflect(builder).release();
            auto cast = dynamic_cast<T *>(ptr);
            Assert(cast, "invalid reflection cast");
            ref = std::unique_ptr<T>(cast);
        }

    protected:
        Location m_Loc;
    };

    class DeleteStatement final : public Statement
    {
    public:
        explicit DeleteStatement(Location loc, ExpressionPtr value);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
    };

    class ForStatement final : public Statement
    {
    public:
        explicit ForStatement(
            Location loc,
            StatementPtr prefix,
            StatementPtr suffix,
            ExpressionPtr condition,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
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
            Location loc,
            bool mutable_,
            bool reference,
            std::string name,
            ExpressionPtr range,
            StatementPtr content);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
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
        explicit IfStatement(Location loc, ExpressionPtr condition, StatementPtr then, StatementPtr else_);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Condition;
        StatementPtr m_Then;
        StatementPtr m_Else;
    };

    class LetStatement final : public Statement
    {
    public:
        explicit LetStatement(
            Location loc,
            Field info,
            std::string name,
            ExpressionPtr value,
            std::vector<ExpressionPtr> arguments);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
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

        explicit ScopeStatement(Location loc, std::vector<StatementPtr> content);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<StatementPtr> m_Content;
    };

    class YieldStatement final : public Statement
    {
    public:
        explicit YieldStatement(Location loc, ExpressionPtr value);

        void Gen(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
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
        explicit Expression(Location loc);
        void Gen(Builder &builder) const override;
        virtual ValuePtr GenVal(Builder &builder, TypePtr expect) const = 0;
        virtual CalleeInfo GenCallee(Builder &builder) const;
    };

    class ArrayExpression final : public Expression
    {
    public:
        explicit ArrayExpression(Location loc, std::vector<ExpressionPtr> values, TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<ExpressionPtr> m_Values;
        TypePtr m_Type;
    };

    class BinaryExpression final : public Expression
    {
    public:
        explicit BinaryExpression(Location loc, std::string operator_, ExpressionPtr left, ExpressionPtr right);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Left;
        ExpressionPtr m_Right;
    };

    class CallExpression final : public Expression
    {
    public:
        explicit CallExpression(Location loc, ExpressionPtr callee, std::vector<ExpressionPtr> arguments);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Callee;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class CreateExpression final : public Expression
    {
    public:
        explicit CreateExpression(
            Location loc,
            TypePtr type,
            ExpressionPtr destination,
            std::vector<ExpressionPtr> arguments);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Type;
        ExpressionPtr m_Destination;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class FloatExpression final : public Expression
    {
    public:
        explicit FloatExpression(Location loc, double_t value, TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        double_t m_Value;
        TypePtr m_Type;
    };

    class IntegerExpression final : public Expression
    {
    public:
        explicit IntegerExpression(Location loc, uint64_t value, TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        uint64_t m_Value;
        TypePtr m_Type;
    };

    class MemberExpression final : public Expression
    {
    public:
        explicit MemberExpression(Location loc, ExpressionPtr value, std::string member);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        CalleeInfo GenCallee(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
        std::string m_Member;
    };

    class NullExpression final : public Expression
    {
    public:
        explicit NullExpression(Location loc, TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Type;
    };

    class RangeExpression final : public Expression
    {
    public:
        explicit RangeExpression(Location loc, ExpressionPtr beg, ExpressionPtr end);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Beg;
        ExpressionPtr m_End;
    };

    class SizeofExpression final : public Expression
    {
    public:
        explicit SizeofExpression(Location loc, TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Type;
    };

    class StringExpression final : public Expression
    {
    public:
        explicit StringExpression(Location loc, std::string value);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Value;
    };

    class StructExpression final : public Expression
    {
    public:
        explicit StructExpression(Location loc, std::map<std::string, ExpressionPtr> values, TypePtr type);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::map<std::string, ExpressionPtr> m_Values;
        TypePtr m_Type;
    };

    class SubscriptExpression final : public Expression
    {
    public:
        explicit SubscriptExpression(Location loc, ExpressionPtr value, ExpressionPtr index);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        ExpressionPtr m_Value;
        ExpressionPtr m_Index;
    };

    class SymbolExpression final : public Expression
    {
    public:
        explicit SymbolExpression(Location loc, std::string name);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        CalleeInfo GenCallee(Builder &builder) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
    };

    class UnaryExpression final : public Expression
    {
    public:
        explicit UnaryExpression(Location loc, std::string operator_, ExpressionPtr operand, bool suffix);

        ValuePtr GenVal(Builder &builder, TypePtr expect) const override;
        StatementPtr Reflect(Builder &builder) const override;
        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Operand;
        bool m_Suffix;
    };

    extern unsigned PrintDepth;
}
