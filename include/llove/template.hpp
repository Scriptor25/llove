#pragma once

#include <llove/type.hpp>
#include <string>
#include <vector>

namespace llove
{
    struct ClassTemplate final
    {
        bool Complete = false;
        bool IsImported = false;

        std::string Name;
        std::vector<std::pair<std::string, TemplateType::Ptr>> TypeParameters;

        std::vector<ClassMember> Members;
        std::vector<ClassFunction> Functions;
    };

    struct DefinitionTemplate final
    {
        bool IsImported = false;

        Location Loc;
        bool IsImplicit;
        std::string Name;
        std::vector<std::pair<std::string, TemplateType::Ptr>> TypeParameters;

        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
        Field Result;

        StatementPtr Content;
    };
}
