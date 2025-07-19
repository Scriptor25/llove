#pragma once

#include <string>
#include <vector>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    struct ClassFieldReference final
    {
        Field Info;
        std::string Name;
    };

    struct ClassFunctionReference final
    {
        bool Expose = false;
        bool Mutable = false;
        std::string Name;
        std::vector<Field> Parameters;
        bool VarArg = false;
        Field Result;
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

    std::ostream &operator<<(std::ostream &stream, const ClassFieldReference &field);
    std::ostream &operator<<(std::ostream &stream, const ClassFunction &function);
}
