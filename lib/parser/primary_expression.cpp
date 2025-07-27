#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParsePrimaryExpression()
{
    auto loc = m_Token.Loc;

    if (At(TokenType_Integer))
    {
        auto value = Skip().IntValue;

        TypePtr type;
        if (SkipIf(TokenType_Other, ":"))
            type = ParseType();

        return std::make_unique<IntExpression>(std::move(loc), value, std::move(type));
    }

    if (At(TokenType_String))
    {
        auto value = Skip().Value;
        return std::make_unique<StringExpression>(std::move(loc), std::move(value));
    }

    if (At(TokenType_Operator, "-", "!", "~", "++", "--", "*", "&", "$"))
    {
        auto operator_ = Skip().Value;
        auto operand = ParseOperandExpression();

        return std::make_unique<UnaryExpression>(std::move(loc), std::move(operator_), std::move(operand), false);
    }

    if (SkipIf(TokenType_Other, "("))
    {
        auto expression = ParseExpression();
        Expect(TokenType_Other, ")");
        return expression;
    }

    if (SkipIf(TokenType_Other, "["))
    {
        std::vector<ExpressionPtr> values;
        while (!At(TokenType_Other, "]"))
        {
            values.emplace_back(ParseExpression());

            if (!At(TokenType_Other, "]"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, "]");

        TypePtr type;
        if (SkipIf(TokenType_Other, ":"))
            type = m_Types.GetArray(ParseType(), values.size());

        return std::make_unique<ArrayExpression>(std::move(loc), std::move(values), std::move(type));
    }

    if (SkipIf(TokenType_Other, "{"))
    {
        std::map<std::string, ExpressionPtr> values;
        while (!At(TokenType_Other, "}"))
        {
            auto name = Expect(TokenType_Symbol).Value;
            Assert(!values.contains(name), "struct expression already has field '{}'", name);

            ExpressionPtr value;
            if (SkipIf(TokenType_Other, ":"))
                value = ParseExpression();
            else
                value = std::make_unique<SymbolExpression>(std::move(loc), name);
            values[name] = std::move(value);

            if (!At(TokenType_Other, "}"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, "}");

        TypePtr type;
        if (SkipIf(TokenType_Other, ":"))
            type = ParseType();

        return std::make_unique<StructExpression>(std::move(loc), std::move(values), std::move(type));
    }

    if (SkipIf(TokenType_Symbol, "null"))
    {
        TypePtr type;
        if (SkipIf(TokenType_Other, ":"))
            type = ParseType();
        return std::make_unique<NullExpression>(std::move(loc), type);
    }

    if (SkipIf(TokenType_Symbol, "create"))
    {
        ExpressionPtr destination;
        if (SkipIf(TokenType_Other, "["))
        {
            destination = ParseExpression();
            Expect(TokenType_Other, "]");
        }

        auto type = ParseType();

        std::vector<ExpressionPtr> arguments;
        if (SkipIf(TokenType_Other, "("))
        {
            while (!At(TokenType_Other, ")"))
            {
                arguments.emplace_back(ParseExpression());

                if (!At(TokenType_Other, ")"))
                    Expect(TokenType_Other, ",");
            }
            Expect(TokenType_Other, ")");
        }

        return std::make_unique<CreateExpression>(
            std::move(loc),
            std::move(type),
            std::move(destination),
            std::move(arguments));
    }

    if (SkipIf(TokenType_Symbol, "sizeof"))
        return std::make_unique<SizeofTypeExpression>(std::move(loc), ParseType());

    if (At(TokenType_Symbol))
        return std::make_unique<SymbolExpression>(std::move(loc), Skip().Value);

    Error("unable to parse expression from {} : '{}'", m_Token.Type, m_Token.Value);
}
