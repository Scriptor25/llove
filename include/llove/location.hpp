#pragma once

#include <filesystem>

namespace llove
{
    struct Location
    {
        bool operator==(const Location &other) const
        {
            return Filepath == other.Filepath && Row == other.Row && Col == other.Col;
        }

        std::filesystem::path Filepath;
        unsigned Row = 0u;
        unsigned Col = 0u;
    };
}

template<>
struct std::hash<llove::Location>
{
    template<typename T>
    static size_t combine(T &&value, const size_t h)
    {
        return std::hash<std::decay_t<T>>()(std::forward<T>(value)) ^ h;
    }

    size_t operator()(const llove::Location &location) const noexcept
    {
        size_t h{};
        h = combine(location.Filepath, h);
        h = combine(location.Row, h);
        h = combine(location.Col, h);
        return h;
    }
};
