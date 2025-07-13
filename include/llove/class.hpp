#pragma once

#include <string>
#include <vector>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    struct ClassField final
    {
        Field Info;
        std::string Name;
    };

    struct ClassFunction final
    {
        bool Expose = false;
        bool Mutable = false;
        std::string Name;
        std::vector<Parameter> Parameters;
        bool VarArg = false;
        Field Result;
        StatementPtr Content;
    };

    struct ClassFunctionInfo final
    {
        bool Expose;
        bool Mutable;
        std::string Name;
        std::vector<Field> Parameters;
        bool VarArg;
        Field Result;
    };

    std::ostream &operator<<(std::ostream &stream, const ClassField &field);
    std::ostream &operator<<(std::ostream &stream, const ClassFunction &function);
}
