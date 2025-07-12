#pragma once

#include <vector>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    struct Type
    {
        virtual ~Type() = default;
    };

    struct VoidType final : Type
    {
        explicit VoidType() = default;
    };

    struct IntType final : Type
    {
        explicit IntType(bool sign, unsigned bits);

        bool Sign;
        unsigned Bits;
    };

    struct FltType final : Type
    {
        explicit FltType(unsigned bits);

        unsigned Bits;
    };

    struct ArrayType final : Type
    {
        explicit ArrayType(TypePtr base, int64_t size);

        TypePtr Base;
        int64_t Size;
    };

    struct StructType final : Type
    {
        explicit StructType(const std::vector<Parameter> &fields);

        std::vector<Parameter> Fields;
    };
}
