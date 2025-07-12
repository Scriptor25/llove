#pragma once

#include <string>
#include <llove/forward.hpp>

namespace llove
{
    struct Field
    {
        bool Mutable;
        bool Reference;
        TypePtr Type;
    };

    struct Parameter
    {
        Field Info;
        std::string Name;
    };

    ParameterHash GetFieldHash(const std::vector<Field> &fields);
}
