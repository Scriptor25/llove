#pragma once

#include <string>
#include <llove/forward.hpp>
#include <llvm/IR/Constant.h>

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
