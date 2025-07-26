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

void llove::Parser::RemoveEscape(std::string &raw, std::string &value)
{
    if (m_Buffer != '\\')
    {
        raw += static_cast<char>(m_Buffer);
        value += static_cast<char>(m_Buffer);
        m_Buffer = m_Stream.get();
        return;
    }

    raw += static_cast<char>(m_Buffer);
    m_Buffer = m_Stream.get();

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
    default:
        value += static_cast<char>(m_Buffer);
        break;
    }

    raw += static_cast<char>(m_Buffer);
    m_Buffer = m_Stream.get();
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
            case '?':
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                return {
                    .Type = TokenType_Otr,
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
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                state = State_Opr;
                break;
            case '"':
                raw += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                state = State_Str;
                break;
            case '\'':
                raw += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                state = State_Chr;
                break;
            case '0':
                raw += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                switch (m_Buffer)
                {
                case 'b':
                    raw += static_cast<char>(m_Buffer);
                    m_Buffer = m_Stream.get();
                    base = 2;
                    flt = false;
                    state = State_Num;
                    break;
                case 'x':
                    raw += static_cast<char>(m_Buffer);
                    m_Buffer = m_Stream.get();
                    base = 16;
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
                    base = 10;
                    flt = false;
                    state = State_Num;
                    break;
                }
                if (isalpha(m_Buffer) || m_Buffer == '_')
                {
                    state = State_Idt;
                    break;
                }
                raw += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            break;
        case State_Cmt:
            if (m_Buffer != '\n')
            {
                raw += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            state = State_Idl;
            break;
        case State_Idt:
            if (isalnum(m_Buffer) || m_Buffer == '_')
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            return {
                .Type = TokenType_Sym,
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
            m_Buffer = m_Stream.get();
            return {
                .Type = TokenType_Str,
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
            m_Buffer = m_Stream.get();
            return {
                .Type = TokenType_Int,
                .Raw = std::move(raw),
                .IntValue = static_cast<uint64_t>(value.at(0)),
            };
        case State_Num:
            if (base == 10 && !flt && m_Buffer == '.')
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                flt = true;
                break;
            }
            if (isdigit(m_Buffer, base))
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            return {
                .Type = flt ? TokenType_Flt : TokenType_Int,
                .Raw = std::move(raw),
                .IntValue = flt ? 0u : std::stoull(value, nullptr, base),
                .FltValue = flt ? std::stod(value) : 0.0,
            };
        case State_Opr:
            if (compound_map.contains(value) && compound_map.at(value).contains(m_Buffer))
            {
                raw += static_cast<char>(m_Buffer);
                value += static_cast<char>(m_Buffer);
                m_Buffer = m_Stream.get();
                break;
            }
            return {
                .Type = TokenType_Opr,
                .Raw = std::move(raw),
                .Value = std::move(value),
            };
        }
    }

    return {
        .Type = TokenType_Eof,
    };
}
