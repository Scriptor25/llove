#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace cli
{
    class Table final
    {
        static constexpr auto PRINT_BORDER = true;

    public:
        explicit Table(std::ostream &stream, unsigned columns, unsigned console_width = 120u, bool ascii = false);
        ~Table();

        Table &operator<<(const std::string &cell);
        Table &operator<<(std::string &&cell);
        Table &operator<<(const char *cell);

    protected:
        void PrintBorder(
            const std::vector<unsigned> &widths,
            std::string &&begin,
            std::string &&cross,
            std::string &&end) const;
        void PrintData(const std::vector<unsigned> &widths, unsigned height, unsigned index) const;
        static std::pair<std::string, unsigned> TrimLine(std::string line, unsigned offset, unsigned max_width);

    private:
        std::ostream &m_Stream;
        unsigned m_Columns;
        unsigned m_MaxColumnWidth;
        bool m_Ascii;

        std::vector<std::string> m_Cells;
    };
}
