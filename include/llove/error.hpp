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

template<typename T>
struct std::formatter<std::optional<T>> : std::formatter<T>
{
    template<typename FormatContext>
    auto format(const std::optional<T> &opt, FormatContext &ctx) const
    {
        if (opt.has_value())
            return std::formatter<T>::format(opt.value(), ctx);
        return std::format_to(ctx.out(), "null");
    }
};

template<typename T, typename A>
struct std::formatter<std::vector<T, A>> : std::formatter<T>
{
    template<typename FormatContext>
    auto format(const std::vector<T, A> &vec, FormatContext &ctx) const
    {
        for (auto i = vec.begin(); i != vec.end(); ++i)
        {
            if (i != vec.begin())
                std::format_to(ctx.out(), ", ");
            std::formatter<T>::format(*i, ctx);
        }
        return ctx.out();
    }
};
