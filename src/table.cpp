#include <ostream>

#include <cli/table.hpp>

template<typename T>
static constexpr T ceil_div(T lhs, T rhs)
{
    return lhs / rhs + (lhs % rhs != 0);
}

cli::Table::Table(std::ostream &stream, const unsigned columns, const unsigned console_width, const bool ascii)
    : m_Stream(stream),
      m_Columns(columns),
      m_MaxColumnWidth(console_width / columns - (3 * columns + 1)),
      m_Ascii(ascii)
{
}

cli::Table::~Table()
{
    std::vector<unsigned> widths(m_Columns);
    std::vector<unsigned> heights(ceil_div<unsigned>(m_Cells.size(), m_Columns));

    for (unsigned i = 0; i < m_Cells.size(); ++i)
    {
        auto &width = widths.at(i % m_Columns);
        auto &height = heights.at(i / m_Columns);

        auto cell_width = m_Cells.at(i).size();
        auto cell_height = 1u;

        if (cell_width > m_MaxColumnWidth)
        {
            cell_height = ceil_div<unsigned>(cell_width, m_MaxColumnWidth);
            cell_width = m_MaxColumnWidth;
        }

        width = std::max<unsigned>(width, cell_width);
        height = std::max<unsigned>(height, cell_height);
    }

    PrintBorder(widths, "┌", "┬", "┐");
    for (unsigned i = 0; i < m_Cells.size(); i += m_Columns)
    {
        if (i)
        {
            PrintBorder(widths, "├", "┼", "┤");
        }
        PrintData(widths, heights.at(i / m_Columns), i);
    }
    PrintBorder(widths, "└", "┴", "┘");
}

cli::Table &cli::Table::operator<<(const std::string &cell)
{
    m_Cells.push_back(cell);
    return *this;
}

cli::Table &cli::Table::operator<<(std::string &&cell)
{
    m_Cells.push_back(cell);
    return *this;
}

cli::Table &cli::Table::operator<<(const char *cell)
{
    m_Cells.push_back(cell);
    return *this;
}

void cli::Table::PrintBorder(
    const std::vector<unsigned> &widths,
    std::string &&begin,
    std::string &&cross,
    std::string &&end) const
{
    if (PRINT_BORDER)
    {
        m_Stream << (m_Ascii ? "+" : begin);
        for (unsigned i = 0; i < m_Columns; ++i)
        {
            if (i)
            {
                m_Stream << (m_Ascii ? "+" : cross);
            }
            for (unsigned j = 0; j < widths.at(i) + 2; ++j)
            {
                m_Stream << (m_Ascii ? "-" : "─");
            }
        }
        m_Stream << (m_Ascii ? "+" : end) << std::endl;
    }
}

void cli::Table::PrintData(const std::vector<unsigned> &widths, const unsigned height, const unsigned index) const
{
    std::vector<unsigned> offsets(m_Columns);

    for (unsigned k = 0; k < height; ++k)
    {
        if (PRINT_BORDER)
        {
            m_Stream << (m_Ascii ? "|" : "│");
        }
        for (unsigned j = 0; j < m_Columns; ++j)
        {
            const auto &cell_data = m_Cells.at(index + j);

            const auto width = widths.at(j);
            auto &cell_offset = offsets.at(j);

            auto [cell, offset] = TrimLine(cell_data, cell_offset, width);
            cell_offset = offset;

            const auto cell_width = cell.size();

            if (PRINT_BORDER)
            {
                m_Stream << ' ' << cell << ' ';
            }
            else
            {
                m_Stream << cell;
            }
            for (unsigned i = cell_width; i < width; ++i)
            {
                m_Stream << ' ';
            }
            if (PRINT_BORDER)
            {
                m_Stream << (m_Ascii ? "|" : "│");
            }
            else
            {
                m_Stream << ' ';
            }
        }
        m_Stream << std::endl;
    }
}

std::pair<std::string, unsigned> cli::Table::TrimLine(std::string line, unsigned offset, const unsigned max_width)
{
    constexpr std::string_view FILTER_TOKEN = " \t\v\n\r";

    if (offset >= line.size())
    {
        return { {}, static_cast<unsigned>(line.size()) };
    }

    while (offset < line.size() && FILTER_TOKEN.find(line.at(offset)) != std::string_view::npos)
    {
        ++offset;
    }

    line = line.substr(offset);

    if (line.empty())
    {
        return { {}, offset };
    }

    if (line.size() <= max_width)
    {
        if (const auto end = line.find_last_not_of(FILTER_TOKEN); end != std::string::npos)
        {
            line = line.substr(0, end + 1);
        }
        return { line, static_cast<unsigned>(offset + line.size()) };
    }

    if (const auto last_space = line.find_last_of(FILTER_TOKEN, max_width);
        last_space == std::string::npos || last_space == 0)
    {
        line = line.substr(0, max_width);
        offset += max_width;
    }
    else
    {
        line = line.substr(0, last_space);
        offset += last_space + 1;
    }

    if (const auto end = line.find_last_not_of(FILTER_TOKEN); end != std::string::npos)
    {
        line = line.substr(0, end + 1);
    }

    return { line, offset };
}
