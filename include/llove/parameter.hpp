#pragma once

#include <string>
#include <llove/forward.hpp>

namespace llvm
{
    class Type;
}

namespace llove
{
    struct Field final
    {
        llvm::Type *Gen(Builder &builder) const;
        std::string Mangle() const;
        std::ostream &Print(std::ostream &stream, bool has_name = false, const std::string &name = {}) const;

        bool Mutable = false;
        bool Reference = false;
        TypePtr Type;
    };

    struct Parameter final
    {
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
    };

    std::string GetFieldHash(const std::vector<Field> &fields);
}
