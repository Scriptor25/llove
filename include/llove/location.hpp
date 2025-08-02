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
