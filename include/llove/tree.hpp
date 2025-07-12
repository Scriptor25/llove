#pragma once

#include <memory>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    struct Global
    {
        virtual ~Global() = default;
    };

    struct DefinitionGlobal final : Global
    {
        explicit DefinitionGlobal(
            bool interface,
            bool demangle,
            std::string name,
            const std::vector<Parameter> &parameters,
            bool vararg,
            Field result,
            StatementPtr content);

        bool Interface;
        bool Demangle;
        std::string Name;
        std::vector<Parameter> Parameters;
        bool Vararg;
        Field Result;
        StatementPtr Content;
    };

    struct Statement
    {
        virtual ~Statement() = default;
    };

    struct ScopeStatement final : Statement
    {
        explicit ScopeStatement(std::vector<StatementPtr> content);

        std::vector<StatementPtr> Content;
    };

    struct LetStatement final : Statement
    {
        explicit LetStatement(Field info, std::string name, ExpressionPtr value);

        Field Info;
        std::string Name;
        ExpressionPtr Value;
    };

    struct ForEachStatement final : Statement
    {
        explicit ForEachStatement(
            bool mutable_,
            bool reference,
            std::string name,
            ExpressionPtr range,
            StatementPtr content);

        bool Mutable;
        bool Reference;
        std::string Name;
        ExpressionPtr Range;
        StatementPtr Content;
    };

    struct YieldStatement final : Statement
    {
        explicit YieldStatement(ExpressionPtr value);

        ExpressionPtr Value;
    };

    struct Expression : Statement
    {
    };

    struct IntExpression final : Expression
    {
        explicit IntExpression(uint64_t value, TypePtr type);

        uint64_t Value;
        TypePtr Type;
    };

    struct StringExpression final : Expression
    {
        explicit StringExpression(std::string value);

        std::string Value;
    };

    struct RangeExpression final : Expression
    {
        explicit RangeExpression(bool include_begin, ExpressionPtr begin, ExpressionPtr end, bool include_end);

        bool IncludeBegin;
        ExpressionPtr Begin;
        ExpressionPtr End;
        bool IncludeEnd;
    };

    struct SymbolExpression final : Expression
    {
        explicit SymbolExpression(std::string name);

        std::string Name;
    };

    struct BinaryExpression final : Expression
    {
        explicit BinaryExpression(std::string operator_, ExpressionPtr left, ExpressionPtr right);

        std::string Operator;
        ExpressionPtr Left;
        ExpressionPtr Right;
    };

    struct UnaryExpression final : Expression
    {
        explicit UnaryExpression(std::string operator_, ExpressionPtr operand, bool suffix);

        std::string Operator;
        ExpressionPtr Operand;
        bool Suffix;
    };

    struct CallExpression final : Expression
    {
        explicit CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> arguments);

        ExpressionPtr Callee;
        std::vector<ExpressionPtr> Arguments;
    };
}
