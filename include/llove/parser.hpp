#pragma once

#include <cmath>
#include <format>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include <llove/class.hpp>
#include <llove/forward.hpp>
#include <llove/location.hpp>

namespace llove
{
    enum TokenType
    {
        TokenType_EndOfFile,
        TokenType_Symbol,
        TokenType_String,
        TokenType_Integer,
        TokenType_Float,
        TokenType_Operator,
        TokenType_Other,
    };

    struct Token
    {
        Location Loc;
        TokenType Type = TokenType_EndOfFile;
        std::string Raw;
        std::string Value;
        uint64_t IntValue = 0;
        double_t FltValue = 0.0;
    };

    class Parser final
    {
    public:
        explicit Parser(Context &types, Builder &builder, std::istream &stream, const std::filesystem::path &filepath);

        [[nodiscard]] bool Ok() const;
        GlobalPtr Parse();

    protected:
        void RemoveEscape(std::string &raw, std::string &value);

        int Get();
        Token Next();
        Token &Pop();

        Token Skip();

        [[nodiscard]] bool At(TokenType type, const std::string &value = {}) const;
        [[nodiscard]] bool At(TokenType type, const std::vector<std::string> &values) const;

        template<typename... Values>
        [[nodiscard]] bool At(const TokenType type, Values... values) const
        {
            return At(type, std::vector<std::string>{ values... });
        }

        bool SkipIf(TokenType type, const std::string &value = {});

        Token Expect(TokenType type, const std::string &value = {});

        TypePtr ParseType();
        TypePtr ParseArrayType();
        TypePtr ParseBaseType();

        std::string ParseField(Field &field, bool require_name = false, bool allow_name = true);

        GlobalPtr ParseGlobal();
        GlobalPtr ParseDefinitionGlobal();
        GlobalPtr ParseClassDefinitionGlobal(Location loc);
        GlobalPtr ParseClassGlobal();

        void ParseClassField(ClassField &field);
        void ParseClassFunction(ClassFunction &function, bool require_content);

        void ParseClassTemplate();

        StatementPtr ParseStatement(bool inline_);
        StatementPtr ParseScopeStatement();
        StatementPtr ParseDeleteStatement(bool inline_);
        StatementPtr ParseForStatement(bool inline_);
        StatementPtr ParseForEachStatement(bool inline_);
        StatementPtr ParseIfStatement(bool inline_);
        StatementPtr ParseLetStatement(bool inline_);
        StatementPtr ParseYieldStatement(bool inline_);

        ExpressionPtr ParseExpression();
        ExpressionPtr ParseBinaryExpression();
        ExpressionPtr ParseBinaryExpression(ExpressionPtr left, unsigned min_precedence);
        ExpressionPtr ParseOperandExpression();
        ExpressionPtr ParsePrimaryExpression();

    private:
        Context &m_Types;
        Builder &m_Builder;

        std::istream &m_Stream;
        int m_Buffer;
        Location m_Loc;
        Token m_Token;
    };
}

template<>
struct std::formatter<llove::TokenType> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const llove::TokenType &type, FormatContext &ctx) const
    {
        static const std::map<llove::TokenType, std::string_view> m
        {
            { llove::TokenType_EndOfFile, "EndOfFile" },
            { llove::TokenType_Symbol, "Symbol" },
            { llove::TokenType_String, "String" },
            { llove::TokenType_Integer, "Integer" },
            { llove::TokenType_Float, "Float" },
            { llove::TokenType_Operator, "Operator" },
            { llove::TokenType_Other, "Other" },
        };
        return std::formatter<std::string_view>::format(m.at(type), ctx);
    }
};
