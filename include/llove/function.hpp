#pragma once

#include <string>
#include <llove/type.hpp>
#include <llvm/IR/Function.h>

namespace llove
{
    struct FunctionReference
    {
        bool Expose;
        std::string Name;
        FunctionType::Ptr Type;
        llvm::Value *Callee;
    };
}
