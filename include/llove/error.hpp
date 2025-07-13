#pragma once

#include <format>
#include <string_view>

namespace llove
{
    template<typename... Args>
    [[noreturn]] void Error(std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw std::runtime_error(message);
    }

    template<typename... Args>
    void Assert(const bool condition, std::string_view format, Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw std::runtime_error(message);
    }
}
