#pragma once

#include <cmath>
#include <format>
#include <iosfwd>
#include <llove/class.hpp>
#include <llove/forward.hpp>
#include <llove/location.hpp>
#include <llove/type.hpp>
#include <map>
#include <set>
#include <string>
#include <vector>

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
        uint64_t IntegerValue = 0;
        double_t FloatValue = 0.0;
    };

    class Parser final
    {
    public:
        explicit Parser(
            Context& context,
            std::istream& stream,
            const std::filesystem::path& filepath,
            const std::set<std::filesystem::path>& includes);

        [[nodiscard]] bool Ok() const;
        GlobalPtr Parse();

    protected:
        void RemoveEscape(
            std::string& raw,
            std::string& value);

        int Get();
        Token Next();
        Token& Pop();

        Token Skip();

        [[nodiscard]] bool At(
            TokenType type,
            const std::string& value = {}) const;
        [[nodiscard]] bool At(
            TokenType type,
            const std::vector<std::string>& values) const;

        template<typename... Values>
        [[nodiscard]] bool At(
            const TokenType type,
            Values... values) const
        {
            return At(type, std::vector<std::string>{ values... });
        }

        bool SkipIf(
            TokenType type,
            const std::string& value = {});

        Token Expect(
            TokenType type,
            const std::string& value = {});
        Token Expect(
            TokenType type,
            const std::vector<std::string>& values);

        template<typename... Values>
        Token Expect(
            const TokenType type,
            Values... values)
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

        std::string ParseField(
            Field& field,
            bool require_name,
            bool require_type);
        void ParseParameter(Parameter& parameter);
        void ParseInitializer(Initializer& initializer);

        Location ParseParameterList(
            std::vector<Parameter>& parameters,
            std::pair<
                bool,
                std::string>& variadic);
        Location ParseTemplateParameterList(
            std::vector<std::pair<
                std::string,
                TemplateType::Ptr>>& parameters);
        Location ParseArgumentList(std::vector<ExpressionPtr>& arguments);

        template<typename T>
        Location ParseList(
            std::vector<T>& list,
            const std::function<void(T& element)>& parse_element,
            const TokenType beg_type,
            const std::string& beg_value,
            const TokenType end_type,
            const std::string& end_value)
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

        template<
            typename T,
            typename E>
        Location ParseList(
            std::vector<T>& list,
            E& ellipsis,
            const std::function<void(T& element)>& parse_element,
            const std::function<void(E& ellipsis)>& parse_ellipsis,
            const TokenType beg_type,
            const std::string& beg_value,
            const TokenType end_type,
            const std::string& end_value,
            const TokenType ellipsis_type,
            const std::string& ellipsis_value)
        {
            auto token = Expect(beg_type, beg_value);
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
            return std::move(token.Loc);
        }

        GlobalPtr ParseGlobal();
        GlobalPtr ParseClassDefinitionGlobal(Location loc);
        GlobalPtr ParseClassGlobal(bool is_export);
        GlobalPtr ParseConstGlobal(bool export_);
        GlobalPtr ParseDefinitionGlobal(bool is_export);
        GlobalPtr ParseImportGlobal();
        GlobalPtr ParseTypeGlobal(bool export_);

        void ParseClassMember(ClassMember& member);
        void ParseClassFunction(
            ClassFunction& function,
            bool require_content);

        void ParseClassTemplate(
            bool is_export,
            std::string name);
        void ParseDefinitionTemplate(
            bool is_export,
            Location loc,
            bool implicit,
            std::string name);

        StatementPtr ParseStatement(bool inline_);
        StatementPtr ParseBreakStatement(bool inline_);
        StatementPtr ParseContinueStatement(bool inline_);
        StatementPtr ParseDeleteStatement(bool inline_);
        StatementPtr ParseForStatement(bool inline_);
        StatementPtr ParseForEachStatement(bool inline_);
        StatementPtr ParseIfStatement(bool inline_);
        StatementPtr ParseLetStatement(bool inline_);
        StatementPtr ParseScopeStatement();
        StatementPtr ParseSwitchStatement();
        StatementPtr ParseWhileStatement(bool inline_);
        StatementPtr ParseRetStatement(bool inline_);

        ExpressionPtr ParseExpression();

        ExpressionPtr ParseArrayExpression();
        ExpressionPtr ParseBinaryExpression();
        ExpressionPtr ParseBinaryExpression(
            ExpressionPtr left,
            unsigned min_precedence);
        ExpressionPtr ParseCallExpression(ExpressionPtr callee);
        ExpressionPtr ParseCastExpression(ExpressionPtr value);
        ExpressionPtr ParseCreateExpression();
        ExpressionPtr ParseFloatExpression();
        ExpressionPtr ParseIntegerExpression();
        ExpressionPtr ParseMemberExpression(ExpressionPtr value);
        ExpressionPtr ParseNullExpression();
        ExpressionPtr ParseOperandExpression();
        ExpressionPtr ParsePrimaryExpression();
        ExpressionPtr ParseRangeExpression(ExpressionPtr begin);
        ExpressionPtr ParseSizeofExpression();
        ExpressionPtr ParseStringExpression();
        ExpressionPtr ParseStructExpression();
        ExpressionPtr ParseSubscriptExpression(ExpressionPtr value);
        ExpressionPtr ParseSwitchExpression();
        ExpressionPtr ParseSymbolExpression();
        ExpressionPtr ParseTemplateCallExpression();
        ExpressionPtr ParseUnaryExpression();
        ExpressionPtr ParseUnaryExpression(ExpressionPtr operand);

    private:
        Context& m_Context;
        const std::set<std::filesystem::path>& m_Includes;

        std::istream& m_Stream;
        int m_Buffer;
        Location m_Loc;
        Token m_Token;
    };
}

template<>
struct std::formatter<llove::TokenType> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(
        const llove::TokenType& type,
        FormatContext& ctx) const
    {
        static const std::map<llove::TokenType, std::string_view> m{
            { llove::TokenType_EndOfFile, "EndOfFile" },
            {    llove::TokenType_Symbol,    "Symbol" },
            {    llove::TokenType_String,    "String" },
            {   llove::TokenType_Integer,   "Integer" },
            {     llove::TokenType_Float,     "Float" },
            {  llove::TokenType_Operator,  "Operator" },
            {     llove::TokenType_Other,     "Other" },
        };
        return std::formatter<std::string_view>::format(m.at(type), ctx);
    }
};
