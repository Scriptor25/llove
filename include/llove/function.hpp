#pragma once

#include <string>

#include <llove/type.hpp>

namespace llove
{
    struct FunctionReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        bool IsPublic{};
        bool IsImplicit{};

        std::string Name;
        FunctionType::Ptr Type;
        llvm::Value *Callee{};
    };

    struct Function final
    {
        Location Loc;

        bool IsExport{};

        bool IsPublic{};
        bool IsVirtual{};
        bool IsOverride{};
        bool IsInterface{};
        bool IsImplicit{};
        bool IsMutable{};

        ClassType::Ptr Class;

        std::string Name;
        std::vector<Parameter> Parameters;
        Variadic Variadic;
        Field Result;

        std::vector<Initializer> Initializers;
        StatementPtr Content;
    };
}

template<>
struct std::formatter<llove::FunctionReference> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const llove::FunctionReference &reference, FormatContext &ctx) const
    {
        std::stringstream stream;
        reference.Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};
