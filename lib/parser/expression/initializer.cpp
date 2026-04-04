#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseInitializerExpression()
{
    auto loc = Expect(TokenType_Other, "{").Loc;
    Assert(!At(TokenType_Other, "}"), loc, "illegal empty initializer expression");

    if (At(TokenType_Operator, "."))
    {
        std::map<std::string, ExpressionPtr> values;
        do
        {
            auto name_loc = Expect(TokenType_Operator, ".").Loc;
            auto name = Expect(TokenType_Symbol).Value;

            Assert(!values.contains(name), name_loc, "struct expression already has field '{}'", name);

            values[name] = SkipIf(TokenType_Operator, "=")
                               ? ParseExpression()
                               : std::make_unique<SymbolExpression>(std::move(loc), name);

            if (!At(TokenType_Other, "}"))
                Expect(TokenType_Other, ",");
        }
        while (!At(TokenType_Other, "}"));
        Expect(TokenType_Other, "}");

        StructType::Ptr type;
        if (SkipIf(TokenType_Operator, ":"))
        {
            auto parsed = ParseType();
            Assert(parsed->IsStruct(), loc, "illegal initializer expression for type '{}'", parsed);
            type = As<StructType>(std::move(parsed));
        }

        return std::make_unique<StructExpression>(std::move(loc), std::move(values), std::move(type));
    }

    std::vector<ExpressionPtr> values;
    do
    {
        values.push_back(ParseExpression());

        if (!At(TokenType_Other, "}"))
            Expect(TokenType_Other, ",");
    }
    while (!At(TokenType_Other, "}"));
    Expect(TokenType_Other, "}");

    TypePtr type;
    if (SkipIf(TokenType_Operator, ":"))
    {
        auto base = ParseType();
        type = m_Context.GetArray(std::move(base), values.size());
    }

    return std::make_unique<ArrayExpression>(std::move(loc), std::move(values), std::move(type));
}
