#pragma once

#include <llove/type.hpp>
#include <string>
#include <vector>

namespace llove
{
    using TypeParameter = std::pair<std::string, TemplateType::Ptr>;

    struct ClassTemplate final
    {
        ClassTemplate() = default;

        ClassTemplate(ClassTemplate&&) = default;
        ClassTemplate& operator=(ClassTemplate&&) = default;

        ClassTemplate(const ClassTemplate&) = delete;
        ClassTemplate& operator=(const ClassTemplate&) = delete;

        bool Complete = false;
        bool IsImported = false;

        std::string Name;
        std::vector<TypeParameter> TypeParameters;

        std::vector<ClassMember> Members;
        std::vector<ClassFunction> Functions;
    };

    struct FunctionTemplate final
    {
        FunctionTemplate() = default;

        FunctionTemplate(FunctionTemplate&&) = default;
        FunctionTemplate& operator=(FunctionTemplate&&) = default;

        FunctionTemplate(const FunctionTemplate&) = delete;
        FunctionTemplate& operator=(const FunctionTemplate&) = delete;

        bool IsImported = false;

        Location Loc;
        bool IsImplicit = false;
        std::string Name;
        std::vector<TypeParameter> TypeParameters;

        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
        Field Result;

        StatementPtr Content;
    };
}
