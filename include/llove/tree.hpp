#pragma once

#include <cmath>
#include <llove/class.hpp>
#include <llove/error.hpp>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/location.hpp>
#include <llove/parameter.hpp>
#include <llove/type.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace llove
{
    class Global
    {
    public:
        explicit Global(Location loc);

        [[nodiscard]] const Location& Loc() const;

        virtual ~Global() = default;
        virtual void Gen(Builder& builder) const = 0;
        virtual std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& context,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const = 0;
        virtual std::ostream& Print(std::ostream& stream) const = 0;

    protected:
        Location m_Loc;
    };

    class ClassGlobal final : public Global
    {
    public:
        explicit ClassGlobal(
            Location loc,
            bool is_export,
            ClassType::Ptr class_type);
        explicit ClassGlobal(
            Location loc,
            bool is_export,
            ClassType::Ptr class_type,
            ClassType::Ptr base_type,
            std::vector<ClassMember> members,
            std::vector<ClassFunction> functions);

        void Gen(Builder& builder) const override;
        std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& context,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        bool m_IsExport;
        bool m_IsOpaque;
        ClassType::Ptr m_ClassType;
        ClassType::Ptr m_BaseType;
        std::vector<ClassMember> m_Members;
        std::vector<ClassFunction> m_Functions;
    };

    class ClassDefinitionGlobal final : public Global
    {
    public:
        explicit ClassDefinitionGlobal(
            Location loc,
            ClassType::Ptr class_type,
            bool is_mutable,
            std::string name,
            std::vector<Parameter> parameters,
            std::pair<
                bool,
                std::string> variadic,
            Field result,
            std::vector<Initializer> initializers,
            StatementPtr content);

        void Gen(Builder& builder) const override;
        std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& context,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ClassType::Ptr m_ClassType;
        bool m_IsMutable;
        std::string m_Name;
        std::vector<Parameter> m_Parameters;
        std::pair<bool, std::string> m_Variadic;
        Field m_Result;
        std::vector<Initializer> m_Initializers;
        StatementPtr m_Content;
    };

    class ConstGlobal final : public Global
    {
    public:
        explicit ConstGlobal(
            Location loc,
            bool export_,
            std::string name,
            TypePtr type,
            ExpressionPtr value);

        void Gen(Builder& builder) const override;
        std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& context,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        bool m_Export;
        std::string m_Name;
        TypePtr m_Type;
        ExpressionPtr m_Value;
    };

    class DefinitionGlobal final : public Global
    {
    public:
        explicit DefinitionGlobal(
            Location loc,
            bool is_export,
            bool is_interface,
            bool is_implicit,
            std::string name,
            std::vector<Parameter> parameters,
            std::pair<
                bool,
                std::string> variadic,
            Field result,
            StatementPtr content);

        void Gen(Builder& builder) const override;
        std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& context,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        bool m_IsExport;
        bool m_IsInterface;
        bool m_IsImplicit;
        std::string m_Name;
        std::vector<Parameter> m_Parameters;
        std::pair<bool, std::string> m_Variadic;
        Field m_Result;
        StatementPtr m_Content;
    };

    class ImportGlobal final : public Global
    {
    public:
        explicit ImportGlobal(
            Location loc,
            std::string as,
            std::map<
                std::string,
                std::string> symbols,
            std::filesystem::path filepath,
            const std::set<std::filesystem::path>& includes);

        void Gen(Builder& builder) const override;
        std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& parent,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_As;
        std::map<std::string, std::string> m_Symbols;
        std::filesystem::path m_Filepath;
        const std::set<std::filesystem::path>& m_Includes;
    };

    class TypeGlobal final : public Global
    {
    public:
        explicit TypeGlobal(
            Location loc,
            bool export_,
            std::string name,
            TypePtr type);

        void Gen(Builder& builder) const override;
        std::pair<
            std::string,
            ValuePtr>
        GenImport(
            Context& context,
            Builder& builder,
            const std::string& as,
            const std::map<
                std::string,
                std::string>& symbols) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        bool m_Export;
        std::string m_Name;
        TypePtr m_Type;
    };

    class Statement
    {
    public:
        explicit Statement(Location loc);

        [[nodiscard]] const Location& Loc() const;

        virtual ~Statement() = default;
        virtual void Gen(Builder& builder) const = 0;
        virtual StatementPtr Reflect(Context& context) const = 0;
        virtual std::ostream& Print(std::ostream& stream) const = 0;

        template<typename T>
        requires std::is_base_of_v<
            Statement,
            T>
        void Reflect(
            Context& context,
            std::unique_ptr<T>& ref) const
        {
            auto ptr = Reflect(context).release();
            auto cast = dynamic_cast<T*>(ptr);
            Assert(cast, "invalid reflection cast");
            ref = std::unique_ptr<T>(cast);
        }

    protected:
        Location m_Loc;
    };

    class BreakStatement final : public Statement
    {
    public:
        explicit BreakStatement(Location loc);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;
    };

    class ContinueStatement final : public Statement
    {
    public:
        explicit ContinueStatement(Location loc);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;
    };

    class DeleteStatement final : public Statement
    {
    public:
        explicit DeleteStatement(
            Location loc,
            ExpressionPtr value);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

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

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

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

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

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
        explicit IfStatement(
            Location loc,
            ExpressionPtr condition,
            StatementPtr then,
            StatementPtr else_);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

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
            Field field,
            std::string name,
            ExpressionPtr value,
            std::vector<ExpressionPtr> arguments);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        Field m_Field;
        std::string m_Name;
        ExpressionPtr m_Value;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class RetStatement final : public Statement
    {
    public:
        explicit RetStatement(
            Location loc,
            ExpressionPtr value);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Value;
    };

    class ScopeStatement final : public Statement
    {
    public:
        static StatementPtr Wrap(StatementPtr ptr);

        explicit ScopeStatement(
            Location loc,
            std::vector<StatementPtr> content);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::vector<StatementPtr> m_Content;
    };

    struct SwitchStatementCase final
    {
        bool IsDefault;
        std::vector<ExpressionPtr> Keys;
        StatementPtr Content;
    };

    class SwitchStatement final : public Statement
    {
    public:
        explicit SwitchStatement(
            Location loc,
            ExpressionPtr condition,
            std::vector<SwitchStatementCase> cases);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Condition;
        std::vector<SwitchStatementCase> m_Cases;
    };

    class WhileStatement final : public Statement
    {
    public:
        explicit WhileStatement(
            Location loc,
            ExpressionPtr condition,
            StatementPtr content);

        void Gen(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Condition;
        StatementPtr m_Content;
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
        void Gen(Builder& builder) const override;
        virtual ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const = 0;
        virtual CalleeInfo GenCallee(Builder& builder) const;
    };

    class ArrayExpression final : public Expression
    {
    public:
        explicit ArrayExpression(
            Location loc,
            std::vector<ExpressionPtr> values,
            ArrayType::Ptr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::vector<ExpressionPtr> m_Values;
        ArrayType::Ptr m_Type;
    };

    class BinaryExpression final : public Expression
    {
    public:
        explicit BinaryExpression(
            Location loc,
            std::string operator_,
            ExpressionPtr left,
            ExpressionPtr right);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Left;
        ExpressionPtr m_Right;
    };

    class CallExpression final : public Expression
    {
    public:
        explicit CallExpression(
            Location loc,
            ExpressionPtr callee,
            std::vector<ExpressionPtr> arguments);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Callee;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class CastExpression final : public Expression
    {
    public:
        explicit CastExpression(
            Location loc,
            ExpressionPtr value,
            TypePtr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Value;
        TypePtr m_Type;
    };

    class CreateExpression final : public Expression
    {
    public:
        explicit CreateExpression(
            Location loc,
            TypePtr type,
            ExpressionPtr destination,
            std::vector<ExpressionPtr> arguments);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        TypePtr m_Type;
        ExpressionPtr m_Destination;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class FloatExpression final : public Expression
    {
    public:
        explicit FloatExpression(
            Location loc,
            double_t value,
            TypePtr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        double_t m_Value;
        TypePtr m_Type;
    };

    using InlineOperand = std::pair<std::string, TypePtr>;

    class InlineExpression final : public Expression
    {
    public:
        explicit InlineExpression(
            Location loc,
            std::string asm_string,
            std::vector<InlineOperand> dst_operands,
            std::vector<InlineOperand> src_operands,
            std::vector<std::string> clobbers,
            bool sideeffect,
            bool alignstack,
            bool inteldialect,
            bool unwind);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_AsmString;
        std::vector<InlineOperand> m_DstOperands, m_SrcOperands;
        std::vector<std::string> m_Clobbers;
        bool m_SideEffect, m_AlignStack, m_IntelDialect, m_Unwind;
    };

    class IntegerExpression final : public Expression
    {
    public:
        explicit IntegerExpression(
            Location loc,
            uint64_t value,
            TypePtr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        uint64_t m_Value;
        TypePtr m_Type;
    };

    class MemberExpression final : public Expression
    {
    public:
        explicit MemberExpression(
            Location loc,
            ExpressionPtr value,
            std::string member,
            bool dereference);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        CalleeInfo GenCallee(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Value;
        std::string m_Member;
        bool m_Dereference;
    };

    class NullExpression final : public Expression
    {
    public:
        explicit NullExpression(
            Location loc,
            PointerType::Ptr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        PointerType::Ptr m_Type;
    };

    class RangeExpression final : public Expression
    {
    public:
        explicit RangeExpression(
            Location loc,
            ExpressionPtr beg,
            ExpressionPtr end);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Beg;
        ExpressionPtr m_End;
    };

    class SizeofExpression final : public Expression
    {
    public:
        explicit SizeofExpression(
            Location loc,
            TypePtr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        TypePtr m_Type;
    };

    class StringExpression final : public Expression
    {
    public:
        explicit StringExpression(
            Location loc,
            std::string value);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Value;
    };

    class StructExpression final : public Expression
    {
    public:
        explicit StructExpression(
            Location loc,
            std::map<
                std::string,
                ExpressionPtr> values,
            TypePtr type);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::map<std::string, ExpressionPtr> m_Values;
        TypePtr m_Type;
    };

    class SubscriptExpression final : public Expression
    {
    public:
        explicit SubscriptExpression(
            Location loc,
            ExpressionPtr value,
            ExpressionPtr index);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Value;
        ExpressionPtr m_Index;
    };

    struct SwitchExpressionCase final
    {
        bool IsDefault = false;
        std::vector<ExpressionPtr> Keys;
        ExpressionPtr Value;
    };

    class SwitchExpression final : public Expression
    {
    public:
        explicit SwitchExpression(
            Location loc,
            ExpressionPtr condition,
            std::vector<SwitchExpressionCase> cases);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Condition;
        std::vector<SwitchExpressionCase> m_Cases;
    };

    class SymbolExpression final : public Expression
    {
    public:
        explicit SymbolExpression(
            Location loc,
            std::string name);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        CalleeInfo GenCallee(Builder& builder) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Name;
    };

    class TemplateCallExpression final : public Expression
    {
    public:
        explicit TemplateCallExpression(
            Location loc,
            std::vector<TypePtr> type_arguments,
            std::string callee,
            std::vector<ExpressionPtr> arguments);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::vector<TypePtr> m_TypeArguments;
        std::string m_Callee;
        std::vector<ExpressionPtr> m_Arguments;
    };

    class TernaryExpression final : public Expression
    {
    public:
        explicit TernaryExpression(
            Location loc,
            ExpressionPtr condition,
            ExpressionPtr then_,
            ExpressionPtr else_);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        ExpressionPtr m_Condition;
        ExpressionPtr m_Then;
        ExpressionPtr m_Else;
    };

    class UnaryExpression final : public Expression
    {
    public:
        explicit UnaryExpression(
            Location loc,
            std::string operator_,
            ExpressionPtr operand,
            bool suffix);

        ValuePtr GenVal(
            Builder& builder,
            TypePtr expect) const override;
        StatementPtr Reflect(Context& context) const override;
        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Operator;
        ExpressionPtr m_Operand;
        bool m_Suffix;
    };

    extern unsigned PrintDepth;
}
