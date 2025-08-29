#include <map>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

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
        { "^=", 0 },
        { "|=", 0 },
        { "&&=", 0 },
        { "||=", 0 },
        { "<<=", 0 },
        { ">>=", 0 },

        { "?", 1 },

        { "||", 2 },

        { "&&", 3 },

        { "|", 4 },

        { "^", 5 },

        { "&", 6 },

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
        return m_Token.Type == TokenType_Operator && map.contains(m_Token.Value);
    };

    auto get_precedence = [this]() -> unsigned
    {
        return map.at(m_Token.Value);
    };

    while (has_precedence() && get_precedence() >= min_precedence)
    {
        const auto operator_precedence = get_precedence();
        auto token = Skip();

        auto right = ParseOperandExpression();
        while (has_precedence()
               && (get_precedence() > operator_precedence
                   || (!get_precedence() && get_precedence() >= operator_precedence)))
            right = ParseBinaryExpression(
                std::move(right),
                operator_precedence + (get_precedence() > operator_precedence ? 1 : 0));

        if (token.Value == "?")
        {
            Expect(TokenType_Operator, ":");
            auto default_ = ParseBinaryExpression();
            left = std::make_unique<TernaryExpression>(
                std::move(token.Loc),
                std::move(left),
                std::move(right),
                std::move(default_));
            continue;
        }

        left = std::make_unique<BinaryExpression>(
            std::move(token.Loc),
            std::move(token.Value),
            std::move(left),
            std::move(right));
    }

    return left;
}
