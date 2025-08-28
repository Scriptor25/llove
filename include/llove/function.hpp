#pragma once

#include <string>
#include <llove/type.hpp>
#include <llvm/IR/Function.h>

namespace llove
{
    struct FunctionReference
    {
        std::ostream &Print(std::ostream &stream) const;

        bool IsExposed = false;
        bool IsImplicit = false;

        std::string Name;
        FunctionType::Ptr Type;
        llvm::Value *Callee = nullptr;
    };

    struct Function final
    {
        Location Loc;

        bool IsExport = false;

        bool IsExposed = false;
        bool IsVirtual = false;
        bool IsOverride = false;
        bool IsInterface = false;
        bool IsImplicit = false;
        bool IsMutable = false;

        ClassType::Ptr Class;

        std::string Name;
        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
        Field Result;

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
