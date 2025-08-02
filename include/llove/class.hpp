#pragma once

#include <string>
#include <vector>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/location.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    struct ClassFieldReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
    };

    struct ClassFunctionReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        bool Expose = false;
        bool Implicit = false;
        bool Delete = false;
        bool Mutable = false;

        std::string Name;
        std::vector<Field> Parameters;
        bool VarArg = false;
        Field Result;
    };

    struct ClassField final
    {
        void Reflect(Builder &builder, ClassField &field) const;
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
        ExpressionPtr Value;
        std::vector<ExpressionPtr> Arguments;
    };

    struct ClassFunction final
    {
        void Reflect(Builder &builder, ClassFunction &function) const;
        std::ostream &Print(std::ostream &stream) const;

        Location Loc;
        bool Expose = false;
        bool Implicit = false;
        bool Delete = false;
        bool Mutable = false;
        std::string Name;
        std::vector<Parameter> Parameters;
        bool VarArg = false;
        Field Result;
        StatementPtr Content;
    };

    std::ostream &operator<<(std::ostream &stream, const ClassField &field);
    std::ostream &operator<<(std::ostream &stream, const ClassFunction &function);
}

template<>
struct std::formatter<llove::ClassFunctionReference> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const llove::ClassFunctionReference &function, FormatContext &ctx) const
    {
        std::stringstream stream;
        function.Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};
