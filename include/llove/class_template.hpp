#pragma once

#include <string>
#include <vector>
#include <llove/type.hpp>

namespace llove
{
    struct ClassTemplate final
    {
        std::string Name;
        std::vector<std::pair<std::string, TemplateType::Ptr>> Parameters;
        std::vector<ClassField> Fields;
        std::vector<ClassFunction> Functions;
    };
}
