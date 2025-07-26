#pragma once

#include <string>
#include <llove/forward.hpp>

namespace llove
{
    struct Location
    {
        std::string Filename;
        unsigned Row = 0u;
        unsigned Col = 0u;
    };
}
