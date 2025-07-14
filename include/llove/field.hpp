#pragma once

#include <string>
#include <llove/forward.hpp>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace llove
{
    struct Field final
    {
        std::ostream &Print(std::ostream &stream, bool has_name = false, const std::string &name = {}) const;

        llvm::Type *Gen(Builder &builder) const;
        llvm::Value *Gen(Builder &builder, ValuePtr value, bool strict = false) const;
        [[nodiscard]] std::string Mangle() const;
        bool operator==(const Field &other) const;

        bool Mutable = false;
        bool Reference = false;
        TypePtr Type;
    };

    std::string GetFieldHash(const std::vector<Field> &fields);
}
