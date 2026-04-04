#pragma once

#include <map>
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

    using Import = std::pair<std::string, ValuePtr>;
    using ImportSymbols = std::map<std::string, std::string>;

    struct Variadic
    {
        bool Is = false;
        std::string Name;
    };

    using GlobalPtr = std::unique_ptr<Global>;
    using StatementPtr = std::unique_ptr<Statement>;
    using ExpressionPtr = std::unique_ptr<Expression>;

    class TemplateInstance;
    using TemplateInstancePtr = std::unique_ptr<TemplateInstance>;

    template<typename T>
    concept type_base = std::is_base_of_v<Type, T>;

    template<typename T>
    concept template_instance_base = std::is_base_of_v<TemplateInstance, T>;

    template<typename T>
    concept global_base = std::is_base_of_v<Global, T>;

    template<typename T>
    concept statement_base = std::is_base_of_v<Statement, T>;

    template<typename T>
    concept expression_base = std::is_base_of_v<Expression, T>;

    std::ostream &operator<<(std::ostream &stream, const Field &field);
    std::ostream &operator<<(std::ostream &stream, const Parameter &parameter);

    template<type_base T>
    std::ostream &operator<<(std::ostream &stream, std::shared_ptr<T> ptr)
    {
        return ptr->Print(stream);
    }

    template<global_base T>
    std::ostream &operator<<(std::ostream &stream, std::shared_ptr<T> ptr)
    {
        return ptr->Print(stream);
    }

    template<statement_base T>
    std::ostream &operator<<(std::ostream &stream, std::shared_ptr<T> ptr)
    {
        if (dynamic_cast<Expression *>(ptr.get()))
            return ptr->Print(stream) << ';';

        return ptr->Print(stream);
    }

    template<expression_base T>
    std::ostream &operator<<(std::ostream &stream, std::shared_ptr<T> ptr)
    {
        return ptr->Print(stream);
    }
}
