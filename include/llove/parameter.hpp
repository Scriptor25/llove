#pragma once

#include <string>
#include <llove/field.hpp>
#include <llove/forward.hpp>

namespace llove
{
    struct Parameter final
    {
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
    };
}
