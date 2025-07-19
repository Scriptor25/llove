#pragma once

#include <string>
#include <llove/forward.hpp>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace llove
{
    struct Penalties final
    {
        unsigned Allocate = 10u;
        unsigned Cast = 5u;
        unsigned NonReference = 1u;
    };

    struct Field final
    {
        /**
         * @param builder builder instance
         * @param dst destination field
         * @param src source field
         * @param error error score reference
         * @param penalties penalties for certain situations
         * @return true if not permitted
         */
        [[nodiscard]] static bool GetCastError(
            const Builder &builder,
            const Field &dst,
            const Field &src,
            unsigned &error,
            const Penalties &penalties = {});
        [[nodiscard]] static bool IsCastable(
            const Builder &builder,
            const Field &dst,
            const Field &src);
        [[nodiscard]] static bool IsAssignable(
            const Field &dst,
            const Field &src);

        std::ostream &Print(std::ostream &stream, bool has_name = false, const std::string &name = {}) const;

        llvm::Type *GenType(Builder &builder) const;
        llvm::Value *GenCast(Builder &builder, ValuePtr value, bool strict = false) const;

        [[nodiscard]] std::string Mangle() const;
        bool operator==(const Field &other) const;

        bool Mutable = false;
        bool Reference = false;
        TypePtr Type;
    };

    std::string GetFieldHash(const std::vector<Field> &fields);
}
