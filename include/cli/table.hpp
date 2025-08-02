#pragma once

#include <iostream>
#include <vector>

namespace cli
{
    template<typename T>
    T CeilDiv(T lhs, T rhs)
    {
        return lhs / rhs + (lhs % rhs != 0);
    }

    class Table final
    {
        static constexpr auto MAX_COLUMN_WIDTH = 37u;
        static constexpr auto PRINT_BORDER = true;

    public:
        explicit Table(std::ostream &stream, unsigned columns);
        ~Table();

        Table &operator<<(const std::string &cell);
        Table &operator<<(std::string &&cell);
        Table &operator<<(const char *cell);

    protected:
        void PrintBorder(const std::vector<unsigned> &widths, std::string &&begin, std::string &&cross, std::string &&end) const;
        void PrintData(const std::vector<unsigned> &widths, unsigned height, unsigned index) const;
        static std::pair<std::string, unsigned> TrimLine(std::string line, unsigned offset, unsigned max_width);

    private:
        std::ostream &m_Stream;
        unsigned m_Columns;
        std::vector<std::string> m_Cells;
    };
}
