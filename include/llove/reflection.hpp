#pragma once

#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/type.hpp>
#include <map>
#include <string>
#include <vector>

namespace llove
{
    struct ClassReflection final
    {
        ClassReflection() = default;

        ClassReflection(ClassReflection&&) = default;
        ClassReflection& operator=(ClassReflection&&) = default;

        ClassReflection(const ClassReflection&) = delete;
        ClassReflection& operator=(const ClassReflection&) = delete;

        std::map<std::string, TypePtr> Frame;

        ClassType::Ptr Class;
        std::vector<ClassFunction> Functions;
    };

    struct DefinitionReflection final
    {
        std::map<std::string, TypePtr> Frame;

        Function Fun;
    };
}
