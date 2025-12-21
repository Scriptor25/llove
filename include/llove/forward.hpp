#pragma once

#include <memory>

namespace llove
{
    class Type;
    using TypePtr = std::shared_ptr<Type>;

    class Value;
    class LValue;
    class RValue;
    using ValuePtr = std::shared_ptr<Value>;

    class Field;
    struct Parameter;

    struct ClassMemberReference;
    struct ClassFunctionReference;
    struct ClassFunction;

    class Global;
    class Statement;
    class Expression;

    class Context;
    class Parser;
    class Builder;
    class DebugBuilder;

    struct Location;

    using GlobalPtr = std::unique_ptr<Global>;
    using StatementPtr = std::unique_ptr<Statement>;
    using ExpressionPtr = std::unique_ptr<Expression>;

    class TemplateInstance;
    using TemplateInstancePtr = std::unique_ptr<TemplateInstance>;

    std::ostream& operator<<(
        std::ostream& stream,
        const Field& field);
    std::ostream& operator<<(
        std::ostream& stream,
        const Parameter& parameter);

    template<typename T>
    requires std::is_base_of_v<
        Type,
        T>
    std::ostream& operator<<(
        std::ostream& stream,
        std::shared_ptr<T> ptr)
    {
        return ptr->Print(stream);
    }

    std::ostream& operator<<(
        std::ostream& stream,
        const GlobalPtr& ptr);
    std::ostream& operator<<(
        std::ostream& stream,
        const StatementPtr& ptr);
    std::ostream& operator<<(
        std::ostream& stream,
        const ExpressionPtr& ptr);
}

template<typename T>
concept TypeLike = std::is_base_of_v<llove::Type, T>;

template<typename T>
concept InstanceLike = std::is_base_of_v<llove::TemplateInstance, T>;
