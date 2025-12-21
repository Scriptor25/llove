#pragma once

#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llvm/IR/Constant.h>
#include <string>

namespace llove
{
    struct Parameter final
    {
        std::ostream& Print(std::ostream& stream) const;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const;

        void Reflect(
            Context& context,
            Parameter& parameter) const;

        Field Info;
        std::string Name;
    };
}
