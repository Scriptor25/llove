#pragma once

#include <format>
#include <sstream>
#include <string>
#include <llove/forward.hpp>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

namespace llove
{
    struct Field final
    {
        /**
         * @param builder builder instance
         * @param dst destination field
         * @param src source field
         * @param error error score reference
         * @param strict
         * @return true if not permitted
         */
        [[nodiscard]] static bool GetCastError(
            const Builder &builder,
            const Field &dst,
            const Field &src,
            unsigned &error,
            bool strict);
        [[nodiscard]] static bool IsCastable(
            const Builder &builder,
            const Field &dst,
            const Field &src,
            bool strict);

        std::ostream &Print(std::ostream &stream, bool has_name = false, const std::string &name = {}) const;

        llvm::Type *GenType(Builder &builder) const;
        llvm::Value *GenCast(Builder &builder, ValuePtr value, bool implicit_ownership = false) const;

        [[nodiscard]] unsigned Size(Builder &builder) const;
        [[nodiscard]] std::string Mangle() const;

        bool operator==(const Field &other) const;
        explicit operator bool() const;

        void Reflect(Context &types, Field& field) const;

        bool Mutable = false;
        bool Reference = false;
        TypePtr Type;
    };

    std::string GetFieldHash(const std::vector<Field> &fields);
}

template<>
struct std::formatter<llove::Field> : std::formatter<std::string_view>
{
    auto format(const llove::Field &field, std::format_context &ctx) const
    {
        std::stringstream stream;
        field.Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};
