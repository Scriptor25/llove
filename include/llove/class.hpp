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

    struct ClassMember final
    {
        void Reflect(Context &context, ClassMember &field) const;
        std::ostream &Print(std::ostream &stream) const;

        Field Info;
        std::string Name;
    };

    struct ClassFunctionReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        bool IsExport{};
        bool IsPublic{};
        bool IsVirtual{};
        bool IsOverride{};
        bool IsImplicit{};
        bool IsMutable{};

        std::string Name;
        std::vector<Field> Parameters;
        bool IsVariadic{};
        Field Result;
    };

    struct Initializer final
    {
        void Reflect(Context &context, Initializer &initializer) const;

        std::string Name;
        ExpressionPtr Value;
        std::vector<ExpressionPtr> Arguments;
    };

    struct ClassFunction final
    {
        void Reflect(Context &context, ClassFunction &function) const;
        std::ostream &Print(std::ostream &stream) const;

        Location Loc;

        bool IsPublic{};
        bool IsVirtual{};
        bool IsOverride{};
        bool IsImplicit{};
        bool IsMutable{};

        std::string Name;
        std::vector<Parameter> Parameters;
        Variadic Variadic;
        Field Result;

        std::vector<Initializer> Initializers;

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
