#pragma once

#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/location.hpp>
#include <llove/parameter.hpp>
#include <string>
#include <vector>

namespace llove
{
    struct ClassMemberReference final
    {
        std::ostream& Print(std::ostream& stream) const;

        Field Info;
        std::string Name;
    };

    struct ClassMember final
    {
        void Reflect(
            Context& context,
            ClassMember& field) const;
        std::ostream& Print(std::ostream& stream) const;

        Field Info;
        std::string Name;
        ExpressionPtr Value;
        std::vector<ExpressionPtr> Arguments;
    };

    struct ClassFunctionReference final
    {
        std::ostream& Print(std::ostream& stream) const;

        bool IsExport = false;
        bool IsExposed = false;
        bool IsVirtual = false;
        bool IsOverride = false;
        bool IsImplicit = false;
        bool IsMutable = false;

        std::string Name;
        std::vector<Field> Parameters;
        bool HasVariadic = false;
        Field Result;
    };

    struct Initializer final
    {
        Initializer() = default;
        Initializer(
            std::string name,
            ExpressionPtr value,
            std::vector<ExpressionPtr> arguments);

        Initializer(Initializer&&) = default;
        Initializer& operator=(Initializer&&) = default;

        Initializer(const Initializer&) = delete;
        Initializer& operator=(const Initializer&) = delete;

        void Reflect(
            Context& context,
            Initializer& initializer) const;

        std::string Name;
        ExpressionPtr Value;
        std::vector<ExpressionPtr> Arguments;
    };

    struct ClassFunction final
    {
        ClassFunction() = default;

        ClassFunction(ClassFunction&&) = default;
        ClassFunction& operator=(ClassFunction&&) = default;

        ClassFunction(const ClassFunction&) = delete;
        ClassFunction& operator=(const ClassFunction&) = delete;

        void Reflect(
            Context& context,
            ClassFunction& function) const;
        std::ostream& Print(std::ostream& stream) const;

        Location Loc;

        bool IsExposed = false;
        bool IsVirtual = false;
        bool IsOverride = false;
        bool IsImplicit = false;
        bool IsMutable = false;

        std::string Name;
        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
        Field Result;

        std::vector<Initializer> Initializers;

        StatementPtr Content;
    };

    std::ostream& operator<<(
        std::ostream& stream,
        const ClassMember& field);
    std::ostream& operator<<(
        std::ostream& stream,
        const ClassFunction& function);
}

template<>
struct std::formatter<llove::ClassFunctionReference> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(
        const llove::ClassFunctionReference& function,
        FormatContext& ctx) const
    {
        std::stringstream stream;
        function.Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};
