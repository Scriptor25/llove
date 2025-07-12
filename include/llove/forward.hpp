#pragma once

#include <memory>

namespace llove
{
    struct Type;

    struct Parameter;
    using ParameterHash = std::string;

    struct Global;
    struct Statement;
    struct Expression;

    class Context;
    class Parser;

    using TypePtr = std::shared_ptr<Type>;
    using GlobalPtr = std::unique_ptr<Global>;
    using StatementPtr = std::unique_ptr<Statement>;
    using ExpressionPtr = std::unique_ptr<Expression>;
}
