#pragma once

#include <format>
#include <string>
#include <string_view>
#include <llove/location.hpp>

namespace llove
{
    class ErrorStack final
    {
    public:
        explicit ErrorStack(const ErrorStack *cause, std::optional<Location> loc, std::optional<std::string> message);

        std::ostream &Print(std::ostream &stream) const;

    private:
        const ErrorStack *m_Cause;
        std::optional<Location> m_Loc;
        std::optional<std::string> m_Message;
    };

    template<typename... Args>
    [[noreturn]] void Error(std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw new ErrorStack(nullptr, std::nullopt, message);
    }

    template<typename... Args>
    void Assert(const bool condition, std::string_view format, Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw new ErrorStack(nullptr, std::nullopt, message);
    }

    template<typename... Args>
    [[noreturn]] void Error(const Location &loc, std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw new ErrorStack(nullptr, loc, message);
    }

    template<typename... Args>
    void Assert(const bool condition, const Location &loc, std::string_view format, Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw new ErrorStack(nullptr, loc, message);
    }

    template<typename... Args>
    [[noreturn]] void Error(const ErrorStack *cause, const Location &loc, std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw new ErrorStack(cause, loc, message);
    }

    template<typename... Args>
    void Assert(
        const bool condition,
        const ErrorStack *cause,
        const Location &loc,
        std::string_view format,
        Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw new ErrorStack(cause, loc, message);
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
        return std::format_to(ctx.out(), "[empty]");
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
            {
                if (i == vec.end() - 1)
                    std::format_to(ctx.out(), " and ");
                else
                    std::format_to(ctx.out(), ", ");
            }
            std::format_to(ctx.out(), "'");
            std::formatter<T>::format(*i, ctx);
            std::format_to(ctx.out(), "'");
        }
        return ctx.out();
    }
};
