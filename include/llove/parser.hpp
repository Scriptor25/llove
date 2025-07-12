#pragma once

#include <cmath>
#include <iosfwd>
#include <string>
#include <vector>
#include <llove/forward.hpp>

namespace llove
{
    enum TokenType
    {
        TokenType_Eof,
        TokenType_Sym,
        TokenType_Str,
        TokenType_Int,
        TokenType_Flt,
        TokenType_Opr,
        TokenType_Otr,
    };

    struct Token
    {
        TokenType Type = TokenType_Eof;
        std::string Raw;
        std::string Value;
        uint64_t IntValue = 0;
        double_t FltValue = 0.0;
    };

    class Parser final
    {
    public:
        explicit Parser(Context &context, std::istream &stream);

        [[nodiscard]] bool Ok() const;
        GlobalPtr Parse();

    protected:
        void Escape(std::string &raw, std::string &value);

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

        StatementPtr ParseStatement();
        StatementPtr ParseScopeStatement();
        StatementPtr ParseLetStatement();
        StatementPtr ParseForEachStatement();
        StatementPtr ParseYieldStatement();

        ExpressionPtr ParseExpression();
        ExpressionPtr ParseBinaryExpression();
        ExpressionPtr ParseBinaryExpression(ExpressionPtr left, unsigned min_precedence);
        ExpressionPtr ParseOperandExpression();
        ExpressionPtr ParsePrimaryExpression();

    private:
        Context &m_Context;
        std::istream &m_Stream;
        int m_Buffer;
        Token m_Token;
    };
}
