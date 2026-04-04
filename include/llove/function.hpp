#pragma once

#include <string>

#include <llove/type.hpp>

namespace llove
{
    struct FunctionReference final
    {
        std::ostream &Print(std::ostream &stream) const;

        bool IsPublic = false;
        bool IsImplicit = false;

        std::string Name;
        FunctionType::Ptr Type;
        llvm::Value *Callee = nullptr;
    };

    struct Function final
    {
        Function() = default;

        Function(Function &&) = default;
        Function &operator=(Function &&) = default;

        Function(const Function &) = delete;
        Function &operator=(const Function &) = delete;

        Location Loc;

        bool IsExport = false;

        bool IsPublic = false;
        bool IsVirtual = false;
        bool IsOverride = false;
        bool IsInterface = false;
        bool IsImplicit = false;
        bool IsMutable = false;

        ClassType::Ptr Class = nullptr;

        std::string Name;
        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
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
