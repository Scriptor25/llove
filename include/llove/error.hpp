#pragma once

#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <llove/location.hpp>

namespace llove
{
    template<typename T>
    class ref_exception final : public std::exception
    {
    public:
        ref_exception() noexcept
            : m_Pointer(nullptr)
        {
        }

        explicit ref_exception(T *pointer) noexcept
            : m_Pointer(pointer)
        {
        }

        explicit ref_exception(T &&value) noexcept
            : m_Pointer(new T(value))
        {
        }

        template<typename... Args>
        explicit ref_exception(Args &&... args) noexcept
            : m_Pointer(new T(std::forward<Args>(args)...))
        {
        }

        ref_exception(const ref_exception &other) = delete;

        ref_exception(ref_exception &&other) noexcept
            : m_Pointer(other.m_Pointer)
        {
            other.m_Pointer = nullptr;
        }

        ref_exception &operator=(const ref_exception &other) = delete;

        ref_exception &operator=(ref_exception &&other) noexcept
        {
            std::swap(m_Pointer, other.m_Pointer);
            return *this;
        }

        ~ref_exception() noexcept override
        {
            delete m_Pointer;
        }

        explicit operator bool() const noexcept
        {
            return m_Pointer != nullptr;
        }

        bool operator!() const noexcept
        {
            return !m_Pointer;
        }

        T *operator->() const noexcept
        {
            return m_Pointer;
        }

        T &operator*() const noexcept
        {
            return *m_Pointer;
        }

    private:
        T *m_Pointer;
    };

    class ErrorStack final
    {
    public:
        explicit ErrorStack(
            ref_exception<ErrorStack> cause,
            std::optional<Location> loc,
            std::optional<std::string> message);

        std::ostream &Print(std::ostream &stream) const;

    private:
        ref_exception<ErrorStack> m_Cause;
        std::optional<Location> m_Loc;
        std::optional<std::string> m_Message;
    };

    template<typename... Args>
    [[noreturn]] void Error(std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw ref_exception<ErrorStack>(ref_exception<ErrorStack>(), std::nullopt, std::move(message));
    }

    template<typename... Args>
    void Assert(const bool condition, std::string_view format, Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw ref_exception<ErrorStack>(ref_exception<ErrorStack>(), std::nullopt, std::move(message));
    }

    template<typename... Args>
    [[noreturn]] void Error(const Location &loc, std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw ref_exception<ErrorStack>(ref_exception<ErrorStack>(), std::move(loc), std::move(message));
    }

    template<typename... Args>
    void Assert(const bool condition, const Location &loc, std::string_view format, Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw ref_exception<ErrorStack>(ref_exception<ErrorStack>(), std::move(loc), std::move(message));
    }

    template<typename... Args>
    [[noreturn]] void Error(ref_exception<ErrorStack> cause, Location loc, std::string_view format, Args &&... args)
    {
        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw ref_exception<ErrorStack>(std::move(cause), std::move(loc), std::move(message));
    }

    template<typename... Args>
    void Assert(
        const bool condition,
        ref_exception<ErrorStack> cause,
        Location loc,
        std::string_view format,
        Args &&... args)
    {
        if (condition)
            return;

        auto message = std::vformat(std::move(format), std::make_format_args(args...));
        throw ref_exception<ErrorStack>(cause, loc, message);
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
