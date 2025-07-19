#include <algorithm>
#include <format>
#include <istream>
#include <map>
#include <set>
#include <vector>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/parameter.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

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

llove::Parser::Parser(Context &types, std::istream &stream)
    : m_Types(types),
      m_Stream(stream)
{
    m_Buffer = stream.get();
    m_Token = Next();
}

bool llove::Parser::Ok() const
{
    return m_Token.Type != TokenType_Eof;
}

llove::GlobalPtr llove::Parser::Parse()
{
    return ParseGlobal();
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

llove::Token &llove::Parser::Pop()
{
    return m_Token = Next();
}

llove::Token llove::Parser::Skip()
{
    auto token = m_Token;
    Pop();
    return token;
}

bool llove::Parser::At(const TokenType type, const std::string &value) const
{
    return m_Token.Type == type && (value.empty() || m_Token.Value == value);
}

bool llove::Parser::At(const TokenType type, const std::vector<std::string> &values) const
{
    if (m_Token.Type != type)
        return false;
    return std::ranges::any_of(
        values,
        [this](auto &value)
        {
            return m_Token.Value == value;
        });
}

bool llove::Parser::SkipIf(const TokenType type, const std::string &value)
{
    if (At(type, value))
    {
        Pop();
        return true;
    }
    return false;
}

llove::Token llove::Parser::Expect(TokenType type, const std::string &value)
{
    Assert(At(type, value), "expected {} : '{}', but is {} : '{}'", type, value, m_Token.Type, m_Token.Value);
    return Skip();
}

llove::TypePtr llove::Parser::ParseType()
{
    return ParseArrayType();
}

llove::TypePtr llove::Parser::ParseArrayType()
{
    auto base = ParseBaseType();
    while (SkipIf(TokenType_Otr, "["))
    {
        if (At(TokenType_Int))
        {
            const auto size = Skip().IntValue;
            base = m_Types.GetArray(std::move(base), size);
        }
        else
        {
            const auto mutable_ = SkipIf(TokenType_Sym, "mut");
            base = m_Types.GetPointer(std::move(base), mutable_);
        }
        Expect(TokenType_Otr, "]");
    }
    return base;
}

llove::TypePtr llove::Parser::ParseBaseType()
{
    if (SkipIf(TokenType_Otr, "{"))
    {
        std::vector<ClassFieldReference> fields;

        while (!At(TokenType_Otr, "}"))
        {
            Field field;
            auto name = ParseField(field, true);

            fields.emplace_back(field, name);

            Expect(TokenType_Otr, ",");
        }
        Expect(TokenType_Otr, "}");

        return m_Types.GetStruct(std::move(fields));
    }

    if (SkipIf(TokenType_Otr, "["))
    {
        const auto mutable_ = SkipIf(TokenType_Sym, "mut");
        Expect(TokenType_Otr, "]");
        return m_Types.GetPointer(mutable_);
    }

    if (SkipIf(TokenType_Sym, "class"))
    {
        Expect(TokenType_Opr, "<");
        auto name = Expect(TokenType_Sym).Value;
        Expect(TokenType_Opr, ">");

        return m_Types.GetClass(std::move(name));
    }

    if (At(TokenType_Sym))
    {
        const auto name = Expect(TokenType_Sym).Value;
        if (auto type = m_Types.Get(name))
            return type;

        if (name == "void")
            return m_Types.GetVoid();
        if (name == "i1")
            return m_Types.GetInteger(true, 1);
        if (name == "i8")
            return m_Types.GetInteger(true, 8);
        if (name == "i16")
            return m_Types.GetInteger(true, 16);
        if (name == "i32")
            return m_Types.GetInteger(true, 32);
        if (name == "i64")
            return m_Types.GetInteger(true, 64);
        if (name == "u1")
            return m_Types.GetInteger(false, 1);
        if (name == "u8")
            return m_Types.GetInteger(false, 8);
        if (name == "u16")
            return m_Types.GetInteger(false, 16);
        if (name == "u32")
            return m_Types.GetInteger(false, 32);
        if (name == "u64")
            return m_Types.GetInteger(false, 64);
        if (name == "f16")
            return m_Types.GetFloat(16);
        if (name == "f32")
            return m_Types.GetFloat(32);
        if (name == "f64")
            return m_Types.GetFloat(64);

        Error("undefined type '{}'", name);
    }

    Error("unable to parse type from {} : '{}'", m_Token.Type, m_Token.Value);
}

std::string llove::Parser::ParseField(Field &field, const bool require_name, const bool allow_name)
{
    field.Mutable = SkipIf(TokenType_Sym, "mut");
    field.Reference = SkipIf(TokenType_Opr, "&");

    auto allow_type = !allow_name;

    std::string name;
    if (allow_name && (require_name || At(TokenType_Sym)))
    {
        name = Expect(TokenType_Sym).Value;
        allow_type = SkipIf(TokenType_Otr, ":");
    }

    if (allow_type)
        field.Type = ParseType();

    return name;
}

llove::GlobalPtr llove::Parser::ParseGlobal()
{
    if (SkipIf(TokenType_Sym, "type"))
    {
        const auto name = Expect(TokenType_Sym).Value;
        Expect(TokenType_Opr, "=");
        auto type = ParseType();
        m_Types.Set(name, std::move(type));
        return nullptr;
    }

    if (At(TokenType_Sym, "define", "interface"))
        return ParseDefinitionGlobal();
    if (At(TokenType_Sym, "class"))
        return ParseClassGlobal();

    Error("unable to parse global from {} : '{}'", m_Token.Type, m_Token.Value);
}

llove::GlobalPtr llove::Parser::ParseDefinitionGlobal()
{
    auto interface = SkipIf(TokenType_Sym, "interface") || (Expect(TokenType_Sym, "define"), false);

    if (!interface && SkipIf(TokenType_Otr, ":"))
        return ParseClassDefinitionGlobal();

    auto name = !interface && At(TokenType_Opr) ? Skip().Value : Expect(TokenType_Sym).Value;

    std::vector<Parameter> parameters;
    auto vararg = false;

    Expect(TokenType_Otr, "(");
    while (!At(TokenType_Otr, ")"))
    {
        if (SkipIf(TokenType_Opr, "..."))
        {
            vararg = true;
            break;
        }

        auto &[info_, name_] = parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Otr, ")"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Otr, ")");

    Field result;
    if (SkipIf(TokenType_Otr, ":"))
        ParseField(result, false, false);
    else
        result.Type = m_Types.GetVoid();

    if (SkipIf(TokenType_Otr, ";"))
        return std::make_unique<DefinitionGlobal>(
            interface,
            std::move(name),
            std::move(parameters),
            vararg,
            std::move(result),
            nullptr);

    auto content = ParseScopeStatement();

    return std::make_unique<DefinitionGlobal>(
        interface,
        std::move(name),
        std::move(parameters),
        vararg,
        std::move(result),
        std::move(content));
}

llove::GlobalPtr llove::Parser::ParseClassDefinitionGlobal()
{
    auto class_name = Expect(TokenType_Sym).Value;
    auto class_type = m_Types.GetClass(class_name);

    auto mutable_ = SkipIf(TokenType_Sym, "mut");
    auto name = At(TokenType_Opr) ? Skip().Value : Expect(TokenType_Sym).Value;

    std::vector<Parameter> parameters;
    auto vararg = false;

    Expect(TokenType_Otr, "(");
    while (!At(TokenType_Otr, ")"))
    {
        if (SkipIf(TokenType_Opr, "..."))
        {
            vararg = true;
            break;
        }

        auto &[info_, name_] = parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Otr, ")"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Otr, ")");

    Field result;
    if (SkipIf(TokenType_Otr, ":"))
        ParseField(result, false, false);
    else
        result.Type = m_Types.GetVoid();

    auto content = ParseScopeStatement();

    return std::make_unique<ClassDefinitionGlobal>(
        std::move(class_type),
        mutable_,
        std::move(name),
        std::move(parameters),
        vararg,
        std::move(result),
        std::move(content));
}

llove::GlobalPtr llove::Parser::ParseClassGlobal()
{
    Expect(TokenType_Sym, "class");
    auto name = Expect(TokenType_Sym).Value;

    const auto type = m_Types.GetClass(name);
    m_Types.Set(name, type);

    if (SkipIf(TokenType_Otr, ";"))
        return std::make_unique<ClassGlobal>(std::move(type));

    std::vector<ClassFieldReference> fields;
    std::vector<ClassFunction> functions;

    Expect(TokenType_Otr, "{");
    while (!At(TokenType_Otr, "}"))
    {
        if (At(TokenType_Sym, "let"))
        {
            ParseClassField(fields.emplace_back());
            continue;
        }

        ParseClassFunction(functions.emplace_back());
    }
    Expect(TokenType_Otr, "}");

    return std::make_unique<ClassGlobal>(std::move(type), std::move(fields), std::move(functions));
}

void llove::Parser::ParseClassField(ClassFieldReference &field)
{
    Expect(TokenType_Sym, "let");
    field.Name = ParseField(field.Info, true);
    Expect(TokenType_Otr, ";");
}

void llove::Parser::ParseClassFunction(ClassFunction &function)
{
    function.Expose = SkipIf(TokenType_Sym, "expose");
    function.Mutable = SkipIf(TokenType_Sym, "mut");
    function.Name = At(TokenType_Opr) ? Skip().Value : Expect(TokenType_Sym).Value;

    Expect(TokenType_Otr, "(");
    while (!At(TokenType_Otr, ")"))
    {
        if (SkipIf(TokenType_Opr, "..."))
        {
            function.VarArg = true;
            break;
        }

        auto &[info_, name_] = function.Parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Otr, ")"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Otr, ")");

    if (SkipIf(TokenType_Otr, ":"))
        ParseField(function.Result, false, false);
    else
        function.Result.Type = m_Types.GetVoid();

    if (SkipIf(TokenType_Otr, ";"))
        return;

    function.Content = ParseScopeStatement();
}

llove::StatementPtr llove::Parser::ParseStatement(const bool inline_)
{
    if (At(TokenType_Otr, "{"))
        return ParseScopeStatement();
    if (At(TokenType_Sym, "for"))
        return ParseForStatement(inline_);
    if (At(TokenType_Sym, "foreach"))
        return ParseForEachStatement(inline_);
    if (At(TokenType_Sym, "if"))
        return ParseIfStatement(inline_);
    if (At(TokenType_Sym, "let"))
        return ParseLetStatement(inline_);
    if (At(TokenType_Sym, "yield"))
        return ParseYieldStatement(inline_);

    auto expression = ParseExpression();
    if (inline_)
        return expression;

    Expect(TokenType_Otr, ";");
    return expression;
}

llove::StatementPtr llove::Parser::ParseScopeStatement()
{
    std::vector<StatementPtr> content;

    Expect(TokenType_Otr, "{");
    while (!At(TokenType_Otr, "}"))
        content.emplace_back(ParseStatement(false));
    Expect(TokenType_Otr, "}");

    return std::make_unique<ScopeStatement>(std::move(content));
}

llove::StatementPtr llove::Parser::ParseForStatement(const bool inline_)
{
    Expect(TokenType_Sym, "for");
    Expect(TokenType_Otr, "(");

    StatementPtr prefix, suffix;
    ExpressionPtr condition;

    if (!SkipIf(TokenType_Otr, ";"))
    {
        prefix = ParseStatement(true);
        Expect(TokenType_Otr, ";");
    }

    if (!SkipIf(TokenType_Otr, ";"))
    {
        condition = ParseExpression();
        Expect(TokenType_Otr, ";");
    }

    if (!SkipIf(TokenType_Otr, ")"))
    {
        suffix = ParseStatement(true);
        Expect(TokenType_Otr, ")");
    }

    auto content = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<ForStatement>(
        std::move(prefix),
        std::move(suffix),
        std::move(condition),
        std::move(content));
}

llove::StatementPtr llove::Parser::ParseForEachStatement(const bool inline_)
{
    Expect(TokenType_Sym, "foreach");
    Expect(TokenType_Otr, "(");

    auto mutable_ = SkipIf(TokenType_Sym, "mut");
    auto reference = SkipIf(TokenType_Opr, "&");
    auto name = Expect(TokenType_Sym).Value;

    Expect(TokenType_Otr, ":");

    auto range = ParseExpression();

    Expect(TokenType_Otr, ")");

    auto content = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<ForEachStatement>(
        mutable_,
        reference,
        std::move(name),
        std::move(range),
        std::move(content));
}

llove::StatementPtr llove::Parser::ParseIfStatement(const bool inline_)
{
    Expect(TokenType_Sym, "if");
    Expect(TokenType_Otr, "(");
    auto condition = ParseExpression();
    Expect(TokenType_Otr, ")");
    auto then = ScopeStatement::Wrap(ParseStatement(inline_));

    StatementPtr else_;
    if (SkipIf(TokenType_Sym, "else"))
        else_ = ScopeStatement::Wrap(ParseStatement(inline_));

    return std::make_unique<IfStatement>(std::move(condition), std::move(then), std::move(else_));
}

llove::StatementPtr llove::Parser::ParseLetStatement(const bool inline_)
{
    Expect(TokenType_Sym, "let");

    Field info;
    auto name = ParseField(info, true);

    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments;

    if (SkipIf(TokenType_Opr, "="))
    {
        value = ParseExpression();
    }
    else if (SkipIf(TokenType_Otr, "("))
    {
        while (!At(TokenType_Otr, ")"))
        {
            arguments.emplace_back(ParseExpression());

            if (!At(TokenType_Otr, ")"))
                Expect(TokenType_Otr, ",");
        }

        Expect(TokenType_Otr, ")");
    }

    if (!inline_)
        Expect(TokenType_Otr, ";");

    return std::make_unique<LetStatement>(std::move(info), std::move(name), std::move(value), std::move(arguments));
}

llove::StatementPtr llove::Parser::ParseYieldStatement(const bool inline_)
{
    Expect(TokenType_Sym, "yield");
    if (!inline_ && SkipIf(TokenType_Otr, ";"))
        return std::make_unique<YieldStatement>(nullptr);

    auto value = ParseExpression();

    if (!inline_)
        Expect(TokenType_Otr, ";");

    return std::make_unique<YieldStatement>(std::move(value));
}

llove::ExpressionPtr llove::Parser::ParseExpression()
{
    return ParseBinaryExpression();
}

llove::ExpressionPtr llove::Parser::ParseBinaryExpression()
{
    return ParseBinaryExpression(ParseOperandExpression(), 0);
}

llove::ExpressionPtr llove::Parser::ParseBinaryExpression(ExpressionPtr left, const unsigned min_precedence)
{
    static const std::map<std::string_view, unsigned> map
    {
        { "=", 0 },
        { "+=", 0 },
        { "-=", 0 },
        { "*=", 0 },
        { "/=", 0 },
        { "%=", 0 },
        { "&=", 0 },
        { "|=", 0 },
        { "^=", 0 },
        { "<<=", 0 },
        { ">>=", 0 },
        { "|", 1 },
        { "^", 2 },
        { "&", 3 },
        { "==", 7 },
        { "!=", 7 },
        { "<", 8 },
        { "<=", 8 },
        { ">", 8 },
        { ">=", 8 },
        { "<<", 9 },
        { ">>", 9 },
        { "+", 10 },
        { "-", 10 },
        { "*", 11 },
        { "/", 11 },
        { "%", 11 },
    };

    auto has_precedence = [this]() -> bool
    {
        return map.contains(m_Token.Value);
    };

    auto get_precedence = [this]() -> unsigned
    {
        return map.at(m_Token.Value);
    };

    while (At(TokenType_Opr) && has_precedence() && get_precedence() >= min_precedence)
    {
        const auto operator_precedence = get_precedence();
        auto [
            type_,
            raw_,
            value_,
            int_,
            flt_
        ] = Skip();

        auto right = ParseOperandExpression();
        while (At(TokenType_Opr) && has_precedence()
               && (get_precedence() > operator_precedence
                   || (!get_precedence() && get_precedence() >= operator_precedence)))
            right = ParseBinaryExpression(
                std::move(right),
                operator_precedence + (get_precedence() > operator_precedence ? 1 : 0));

        left = std::make_unique<BinaryExpression>(std::move(value_), std::move(left), std::move(right));
    }

    return left;
}

llove::ExpressionPtr llove::Parser::ParseOperandExpression()
{
    auto expression = ParsePrimaryExpression();

    while (true)
    {
        if (SkipIf(TokenType_Otr, "("))
        {
            std::vector<ExpressionPtr> arguments;

            while (!At(TokenType_Otr, ")"))
            {
                arguments.emplace_back(ParseExpression());

                if (!At(TokenType_Otr, ")"))
                    Expect(TokenType_Otr, ",");
            }
            Expect(TokenType_Otr, ")");

            expression = std::make_unique<CallExpression>(std::move(expression), std::move(arguments));
            continue;
        }

        if (SkipIf(TokenType_Opr, "."))
        {
            auto member = Expect(TokenType_Sym).Value;

            expression = std::make_unique<MemberExpression>(std::move(expression), std::move(member));
            continue;
        }

        if (SkipIf(TokenType_Opr, ".."))
        {
            auto end = ParsePrimaryExpression();

            expression = std::make_unique<RangeExpression>(std::move(expression), std::move(end));
            continue;
        }

        if (SkipIf(TokenType_Otr, "["))
        {
            auto index = ParseExpression();
            Expect(TokenType_Otr, "]");

            expression = std::make_unique<SubscriptExpression>(std::move(expression), std::move(index));
            continue;
        }

        if (At(TokenType_Opr, "++", "--"))
        {
            auto operator_ = Skip().Value;

            expression = std::make_unique<UnaryExpression>(std::move(operator_), std::move(expression), true);
            continue;
        }

        return expression;
    }
}

llove::ExpressionPtr llove::Parser::ParsePrimaryExpression()
{
    if (At(TokenType_Int))
    {
        auto value = Skip().IntValue;

        IntegerType::Ptr type;
        if (SkipIf(TokenType_Otr, ":"))
        {
            type = As<IntegerType>(ParseType());
            Assert(type != nullptr, "expected integer type");
        }

        return std::make_unique<IntExpression>(value, std::move(type));
    }

    if (At(TokenType_Str))
    {
        auto value = Skip().Value;
        return std::make_unique<StringExpression>(std::move(value));
    }

    if (At(TokenType_Opr, "-", "!", "~", "++", "--", "*", "&"))
    {
        auto operator_ = Skip().Value;
        auto operand = ParseOperandExpression();

        return std::make_unique<UnaryExpression>(std::move(operator_), std::move(operand), false);
    }

    if (SkipIf(TokenType_Otr, "("))
    {
        auto expression = ParseExpression();
        Expect(TokenType_Otr, ")");
        return expression;
    }

    if (SkipIf(TokenType_Otr, "["))
    {
        std::vector<ExpressionPtr> values;
        while (!At(TokenType_Otr, "]"))
        {
            values.emplace_back(ParseExpression());

            if (!At(TokenType_Otr, "]"))
                Expect(TokenType_Otr, ",");
        }
        Expect(TokenType_Otr, "]");

        ArrayType::Ptr type;
        if (SkipIf(TokenType_Otr, ":"))
        {
            type = As<ArrayType>(ParseType());
            Assert(type != nullptr, "expected array type");
        }

        return std::make_unique<ArrayExpression>(std::move(values), std::move(type));
    }

    if (SkipIf(TokenType_Otr, "{"))
    {
        std::map<std::string, ExpressionPtr> values;
        while (!At(TokenType_Otr, "}"))
        {
            auto name = Expect(TokenType_Sym).Value;
            Assert(!values.contains(name), "struct expression must not set field '{}' twice", name);

            ExpressionPtr value;
            if (SkipIf(TokenType_Otr, ":"))
                value = ParseExpression();
            else
                value = std::make_unique<SymbolExpression>(name);
            values[name] = std::move(value);

            if (!At(TokenType_Otr, "}"))
                Expect(TokenType_Otr, ",");
        }
        Expect(TokenType_Otr, "}");

        StructType::Ptr type;
        if (SkipIf(TokenType_Otr, ":"))
        {
            type = As<StructType>(ParseType());
            Assert(type != nullptr, "expected struct type");
        }

        return std::make_unique<StructExpression>(std::move(values), std::move(type));
    }

    if (SkipIf(TokenType_Sym, "null"))
    {
        TypePtr type;
        if (SkipIf(TokenType_Otr, ":"))
            type = ParseType();
        return std::make_unique<NullExpression>(type);
    }

    if (At(TokenType_Sym))
    {
        auto name = Skip().Value;
        return std::make_unique<SymbolExpression>(std::move(name));
    }

    Error("unable to parse expression from {} : '{}'", m_Token.Type, m_Token.Value);
}
