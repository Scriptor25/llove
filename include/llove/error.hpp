#pragma once

#include <format>
#include <string_view>

namespace llove
{
    template<typename... Args>
    [[noreturn]] void Error(std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(std::forward<Args>(args)...));
        throw std::runtime_error(message);
    }
}
