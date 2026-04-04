#pragma once

#include <cmath>
#include <format>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <llove/forward.hpp>
#include <llove/location.hpp>
#include <llove/type.hpp>

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

    struct Token final
    {
        Location Loc;
        TokenType Type;
        std::string Raw{};
        std::string Value{};
        uint64_t IntegerValue{};
        double_t FloatValue{};
    };

    class Parser final
    {
    public:
        explicit Parser(
            Context &context,
            std::istream &stream,
            const std::filesystem::path &filepath,
            const std::set<std::filesystem::path> &includes);

        bool Ok() const;
        GlobalPtr Parse();

    protected:
        void RemoveEscape(std::string &raw, std::string &value);

        int Get();
        Token Next();
        Token &Pop();

        Token Skip();

        bool At(TokenType type, const std::string &value = {}) const;
        bool At(TokenType type, const std::vector<std::string> &values) const;

        template<typename... Values>
        bool At(const TokenType type, Values... values) const
        {
            return At(type, std::vector<std::string>{ values... });
        }

        bool SkipIf(TokenType type, const std::string &value = {});

        Token Expect(TokenType type, const std::string &value = {});
        Token Expect(TokenType type, const std::vector<std::string> &values);

        template<typename... Values>
        Token Expect(const TokenType type, Values... values)
        {
            return Expect(type, std::vector<std::string>{ values... });
        }

        TypePtr ParseType();

        TypePtr ParseArrayType();
        TypePtr ParseBaseType();
        TypePtr ParseClassType();
        TypePtr ParseFunctionType();
        TypePtr ParseNamedType();
        TypePtr ParsePointerType();
        TypePtr ParseRangeType();
        TypePtr ParseStructType();
        TypePtr ParseTemplateType();

        std::string ParseField(Field &field, bool require_name, bool require_type);
        void ParseParameter(Parameter &parameter);
        void ParseInitializer(Initializer &initializer);

        Location ParseParameterList(std::vector<Parameter> &parameters, Variadic &variadic);
        Location ParseTemplateParameterList(std::vector<std::pair<std::string, TemplateType::Ptr>> &parameters);
        Location ParseArgumentList(std::vector<ExpressionPtr> &arguments);

        template<typename T>
        Location ParseList(
            std::vector<T> &list,
            const std::function<void(T &element)> &parse_element,
            const TokenType beg_type,
            const std::string &beg_value,
            const TokenType end_type,
            const std::string &end_value)
        {
            auto token = Expect(beg_type, beg_value);
            while (!At(end_type, end_value))
            {
                parse_element(list.emplace_back());

                if (!At(end_type, end_value))
                    Expect(TokenType_Other, ",");
            }
            Expect(end_type, end_value);
            return std::move(token.Loc);
        }

        template<typename T, typename E>
        Location ParseList(
            std::vector<T> &list,
            E &ellipsis,
            const std::function<void(T &element)> &parse_element,
            const std::function<void(E &ellipsis)> &parse_ellipsis,
            const TokenType beg_type,
            const std::string &beg_value,
            const TokenType end_type,
            const std::string &end_value,
            const TokenType ellipsis_type,
            const std::string &ellipsis_value)
        {
            auto loc = Expect(beg_type, beg_value).Loc;
            while (!At(end_type, end_value))
            {
                if (SkipIf(ellipsis_type, ellipsis_value))
                {
                    parse_ellipsis(ellipsis);
                    break;
                }

                parse_element(list.emplace_back());

                if (!At(end_type, end_value))
                    Expect(TokenType_Other, ",");
            }
            Expect(end_type, end_value);
            return loc;
        }

        GlobalPtr ParseGlobal(bool is_template);

        GlobalPtr ParseImportGlobal();

        GlobalPtr ParseConstGlobal(bool is_export);
        GlobalPtr ParseLetGlobal(bool is_export);
        GlobalPtr ParseTemplateGlobal(bool is_export);

        GlobalPtr ParseClassGlobal(bool is_template, bool is_export);
        GlobalPtr ParseFunctionGlobal(bool is_template, bool is_export);
        GlobalPtr ParseTypeGlobal(bool is_template, bool is_export);

        GlobalPtr ParseClassFunctionGlobal(Location loc);

        void ParseClassMember(ClassMember &member);
        void ParseClassFunction(ClassFunction &function, bool require_content);

        StatementPtr ParseStatement(bool is_inline);
        StatementPtr ParseBreakStatement(bool is_inline);
        StatementPtr ParseContinueStatement(bool is_inline);
        StatementPtr ParseDeleteStatement(bool is_inline);
        StatementPtr ParseForStatement(bool is_inline);
        StatementPtr ParseForEachStatement(bool is_inline);
        StatementPtr ParseIfStatement(bool is_inline);
        StatementPtr ParseLetStatement(bool is_inline);
        StatementPtr ParseRetStatement(bool is_inline);
        StatementPtr ParseScopeStatement();
        StatementPtr ParseSwitchStatement();
        StatementPtr ParseWhileStatement(bool is_inline);

        ExpressionPtr ParseExpression();

        ExpressionPtr ParseLambdaExpression();
        ExpressionPtr ParseBinaryExpression();
        ExpressionPtr ParseBinaryExpression(ExpressionPtr left, unsigned min_precedence);
        ExpressionPtr ParseCallExpression(ExpressionPtr callee);
        ExpressionPtr ParseCastExpression(ExpressionPtr value);
        ExpressionPtr ParseCreateExpression();
        ExpressionPtr ParseFloatExpression();
        ExpressionPtr ParseInlineExpression();
        ExpressionPtr ParseIntegerExpression();
        ExpressionPtr ParseMemberExpression(ExpressionPtr value);
        ExpressionPtr ParseNullExpression();
        ExpressionPtr ParseOperandExpression();
        ExpressionPtr ParsePrimaryExpression();
        ExpressionPtr ParseRangeExpression(ExpressionPtr begin);
        ExpressionPtr ParseSizeofExpression();
        ExpressionPtr ParseStringExpression();
        ExpressionPtr ParseInitializerExpression();
        ExpressionPtr ParseSubscriptExpression(ExpressionPtr value);
        ExpressionPtr ParseSwitchExpression();
        ExpressionPtr ParseSymbolExpression();
        ExpressionPtr ParseTemplateCallExpression();
        ExpressionPtr ParseUnaryExpression();
        ExpressionPtr ParseUnaryExpression(ExpressionPtr operand);

    private:
        Context &m_Context;
        const std::set<std::filesystem::path> &m_Includes;

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
        static const std::map<llove::TokenType, std::string_view> map
        {
            { llove::TokenType_EndOfFile, "EndOfFile" },
            { llove::TokenType_Symbol, "Symbol" },
            { llove::TokenType_String, "String" },
            { llove::TokenType_Integer, "Integer" },
            { llove::TokenType_Float, "Float" },
            { llove::TokenType_Operator, "Operator" },
            { llove::TokenType_Other, "Other" },
        };
        return std::formatter<std::string_view>::format(map.at(type), ctx);
    }
};
