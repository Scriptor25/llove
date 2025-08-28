#include <istream>
#include <map>
#include <set>
#include <llove/parser.hpp>

static bool isdigit(const int c, const int base)
{
    switch (base)
    {
    case 2:
        return '0' <= c && c <= '1';
    case 8:
        return '0' <= c && c <= '7';
    case 10:
        return '0' <= c && c <= '9';
    case 16:
        return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
    default:
        return false;
    }
}

static unsigned ctoi(const int c)
{
    if ('A' <= c && c <= 'F')
        return c - 'A' + 0xA;
    if ('a' <= c && c <= 'f')
        return c - 'a' + 0xa;
    return c - '0';
}

void llove::Parser::RemoveEscape(std::string &raw, std::string &value)
{
    if (m_Buffer != '\\')
    {
        raw += static_cast<char>(m_Buffer);
        value += static_cast<char>(m_Buffer);
        Get();
        return;
    }

    raw += static_cast<char>(m_Buffer);
    Get();

    switch (m_Buffer)
    {
    case 'a':
        value += '\a';
        break;
    case 'b':
        value += '\b';
        break;
    case 'f':
        value += '\f';
        break;
    case 'n':
        value += '\n';
        break;
    case 'r':
        value += '\r';
        break;
    case 't':
        value += '\t';
        break;
    case 'v':
        value += '\v';
        break;
    case 'x':
    {
        unsigned buffer = 0;

        raw += static_cast<char>(m_Buffer);
        Get();
        buffer = (ctoi(m_Buffer) & 15) << 4;
        raw += static_cast<char>(m_Buffer);
        Get();
        buffer |= ctoi(m_Buffer) & 15;
        raw += static_cast<char>(m_Buffer);

        value += static_cast<char>(buffer);

        break;
    }
    default:
        value += static_cast<char>(m_Buffer);
        break;
    }

    raw += static_cast<char>(m_Buffer);
    Get();
}

int llove::Parser::Get()
{
    m_Buffer = m_Stream.get();

    if (m_Buffer == '\n')
    {
        m_Loc.Col = 0;
        m_Loc.Row++;
    }
    else
    {
        m_Loc.Col++;
    }

    return m_Buffer;
}

llove::Token llove::Parser::Next()
{
    static const std::map<std::string, std::set<int>> compound_map
    {
        { "+", { '=', '+' } },
        { "-", { '=', '-', '>' } },
        { "*", { '=' } },
        { "/", { '=' } },
        { "%", { '=' } },
        { "&", { '=' } },
        { "|", { '=' } },
        { "^", { '=' } },
        { "=", { '=', '>' } },
        { "<", { '=', '<' } },
        { ">", { '=', '>' } },
        { "!", { '=' } },
        { "~", { '=' } },
        { ".", { '.' } },
        { "..", { '.' } },
    };

    enum
    {
        State_Idl,
        State_Cmt,
        State_Idt,
        State_Str,
        State_Chr,
        State_Num,
        State_Opr,
    } state = State_Idl;

    std::string raw, value;
    auto base = 0;
    auto flt = false;

    Location loc;

    while (m_Buffer >= 0)
    {
        switch (state)
        {
        case State_Idl:
            switch (m_Buffer)
            {
            case '#':
                state = State_Cmt;
                break;
            case '(':
            case ')':
            case '{':
            case '}':
            case '[':
            case ']':
            case ':':
            case ',':
            case ';':
            case '@':
                loc = m_Loc;
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                Get();
                return {
                    .Loc = std::move(loc),
                    .Type = TokenType_Other,
                    .Raw = std::move(raw),
                    .Value = std::move(value),
                };
            case '.':
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '&':
            case '|':
            case '^':
            case '=':
            case '<':
            case '>':
            case '!':
            case '~':
            case '$':
            case '?':
                loc = m_Loc;
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                Get();
                state = State_Opr;
                break;
            case '"':
                loc = m_Loc;
                raw += static_cast<char>(m_Buffer);
                Get();
                state = State_Str;
                break;
            case '\'':
                loc = m_Loc;
                raw += static_cast<char>(m_Buffer);
                Get();
                state = State_Chr;
                break;
            case '0':
                loc = m_Loc;
                raw += static_cast<char>(m_Buffer);
                Get();
                switch (m_Buffer)
                {
                case 'b':
                    raw += static_cast<char>(m_Buffer);
                    Get();
                    base = 2;
                    flt = false;
                    state = State_Num;
                    break;
                case 'x':
                    raw += static_cast<char>(m_Buffer);
                    Get();
                    base = 16;
                    flt = false;
                    state = State_Num;
                    break;
                case '.':
                    value += '0';
                    base = 10;
                    flt = false;
                    state = State_Num;
                    break;
                default:
                    value += '0';
                    base = 8;
                    flt = false;
                    state = State_Num;
                    break;
                }
                break;
            default:
                if (isdigit(m_Buffer))
                {
                    loc = m_Loc;
                    base = 10;
                    flt = false;
                    state = State_Num;
                    break;
                }
                if (isalpha(m_Buffer) || m_Buffer == '_')
                {
                    loc = m_Loc;
                    state = State_Idt;
                    break;
                }
                raw += static_cast<char>(m_Buffer);
                Get();
                break;
            }
            break;
        case State_Cmt:
            if (m_Buffer != '\n')
            {
                raw += static_cast<char>(m_Buffer);
                Get();
                break;
            }
            state = State_Idl;
            break;
        case State_Idt:
            if (isalnum(m_Buffer) || m_Buffer == '_')
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }
            return {
                .Loc = std::move(loc),
                .Type = TokenType_Symbol,
                .Raw = std::move(raw),
                .Value = std::move(value),
            };
        case State_Str:
            if (m_Buffer != '"')
            {
                RemoveEscape(raw, value);
                break;
            }
            raw += static_cast<char>(m_Buffer);
            Get();
            return {
                .Loc = std::move(loc),
                .Type = TokenType_String,
                .Raw = std::move(raw),
                .Value = std::move(value),
            };
        case State_Chr:
            if (m_Buffer != '\'')
            {
                RemoveEscape(raw, value);
                break;
            }
            raw += static_cast<char>(m_Buffer);
            Get();
            return {
                .Loc = std::move(loc),
                .Type = TokenType_Integer,
                .Raw = std::move(raw),
                .IntegerValue = static_cast<uint64_t>(value.at(0)),
            };
        case State_Num:
            if (base == 10 && !flt && m_Buffer == '.')
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                Get();
                flt = true;
                break;
            }
            if (isdigit(m_Buffer, base))
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }
            return {
                .Loc = std::move(loc),
                .Type = flt ? TokenType_Float : TokenType_Integer,
                .Raw = std::move(raw),
                .IntegerValue = flt ? 0u : std::stoull(value, nullptr, base),
                .FloatValue = flt ? std::stod(value) : 0.0,
            };
        case State_Opr:
            if (compound_map.contains(value) && compound_map.at(value).contains(m_Buffer))
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                Get();
                break;
            }
            return {
                .Loc = std::move(loc),
                .Type = TokenType_Operator,
                .Raw = std::move(raw),
                .Value = std::move(value),
            };
        }
    }

    return {
        .Loc = std::move(loc),
        .Type = TokenType_EndOfFile,
    };
}
