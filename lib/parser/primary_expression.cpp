#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParsePrimaryExpression()
{
    if (At(TokenType_Int))
    {
        auto value = Skip().IntValue;

        TypePtr type;
        if (SkipIf(TokenType_Otr, ":"))
            type = ParseType();

        return std::make_unique<IntExpression>(value, std::move(type));
    }

    if (At(TokenType_Str))
    {
        auto value = Skip().Value;
        return std::make_unique<StringExpression>(std::move(value));
    }

    if (At(TokenType_Opr, "-", "!", "~", "++", "--", "*", "&", "$"))
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

        TypePtr type;
        if (SkipIf(TokenType_Otr, ":"))
            type = m_Types.GetArray(ParseType(), values.size());

        return std::make_unique<ArrayExpression>(std::move(values), std::move(type));
    }

    if (SkipIf(TokenType_Otr, "{"))
    {
        std::map<std::string, ExpressionPtr> values;
        while (!At(TokenType_Otr, "}"))
        {
            auto name = Expect(TokenType_Sym).Value;
            Assert(!values.contains(name), "struct expression already has field '{}'", name);

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

        TypePtr type;
        if (SkipIf(TokenType_Otr, ":"))
            type = ParseType();

        return std::make_unique<StructExpression>(std::move(values), std::move(type));
    }

    if (SkipIf(TokenType_Sym, "null"))
    {
        TypePtr type;
        if (SkipIf(TokenType_Otr, ":"))
            type = ParseType();
        return std::make_unique<NullExpression>(type);
    }

    if (SkipIf(TokenType_Sym, "sizeof"))
    {
        Expect(TokenType_Opr, "<");
        auto type = ParseType();
        Expect(TokenType_Opr, ">");
        return std::make_unique<SizeofTypeExpression>(std::move(type));
    }

    if (At(TokenType_Sym))
    {
        auto name = Skip().Value;
        return std::make_unique<SymbolExpression>(std::move(name));
    }

    Error("unable to parse expression from {} : '{}'", m_Token.Type, m_Token.Value);
}
