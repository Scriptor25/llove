#pragma once

#include <string>
#include <vector>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/location.hpp>
#include <llove/parameter.hpp>

namespace llove
{
    struct ClassMemberReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
    };

    struct ClassFunctionReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        bool Expose = false;
        bool Virtual = false;
        bool Override = false;
        bool Implicit = false;
        bool IsMutable = false;

        std::string Name;
        std::vector<Field> Parameters;
        bool IsVariadic = false;
        Field Result;
    };

    struct ClassMember final
    {
        void Reflect(Context &context, ClassMember &field) const;
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
        ExpressionPtr Value;
        std::vector<ExpressionPtr> Arguments;
    };

    struct ClassFunction final
    {
        void Reflect(Context &context, ClassFunction &function) const;
        std::ostream &Print(std::ostream &stream) const;

        Location Loc;
        bool Expose = false;
        bool Virtual = false;
        bool Override = false;
        bool Implicit = false;
        bool Mutable = false;
        std::string Name;
        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
        Field Result;
        StatementPtr Content;
    };

    std::ostream &operator<<(std::ostream &stream, const ClassMember &field);
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
