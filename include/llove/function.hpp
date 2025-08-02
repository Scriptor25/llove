#pragma once

#include <string>
#include <llove/type.hpp>
#include <llvm/IR/Function.h>

namespace llove
{
    struct FunctionReference
    {
        std::ostream &Print(std::ostream &stream) const;

        bool Expose = false;
        bool Implicit = false;
        bool Delete = false;
        std::string Name;
        FunctionType::Ptr Type;
        llvm::Value *Callee = nullptr;
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
