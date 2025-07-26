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

    struct Field;
    struct Parameter;

    struct ClassFieldReference;
    struct ClassFunctionReference;
    struct ClassFunction;

    class Global;
    class Statement;
    class Expression;

    class Context;
    class Parser;
    class Builder;

    struct Location;

    using GlobalPtr = std::unique_ptr<Global>;
    using StatementPtr = std::unique_ptr<Statement>;
    using ExpressionPtr = std::unique_ptr<Expression>;

    std::ostream &operator<<(std::ostream &stream, const Field &field);
    std::ostream &operator<<(std::ostream &stream, const Parameter &parameter);

    template<typename T> requires std::is_base_of_v<Type, T>
    std::ostream &operator<<(std::ostream &stream, const std::shared_ptr<T> &ptr)
    {
        return ptr->Print(stream);
    }

    std::ostream &operator<<(std::ostream &stream, const GlobalPtr &ptr);
    std::ostream &operator<<(std::ostream &stream, const StatementPtr &ptr);
    std::ostream &operator<<(std::ostream &stream, const ExpressionPtr &ptr);
}
