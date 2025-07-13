#pragma once

#include <string>
#include <llove/forward.hpp>
#include <llvm/IR/Type.h>

namespace llove
{
    struct Field final
    {
        llvm::Type *Gen(Builder &builder) const;
        std::string Mangle() const;
        std::ostream &Print(std::ostream &stream, bool has_name = false, const std::string &name = {}) const;

        bool operator==(const Field &other) const;

        bool Mutable = false;
        bool Reference = false;
        TypePtr Type;
    };

    std::string GetFieldHash(const std::vector<Field> &fields);
}
