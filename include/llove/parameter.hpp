#pragma once

#include <string>
#include <llove/field.hpp>
#include <llove/forward.hpp>

namespace llove
{
    struct Parameter final
    {
        std::ostream &Print(std::ostream &stream) const;
        bool TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const;

        Field Info;
        std::string Name;
    };
}
