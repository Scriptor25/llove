#include <llove/error.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseStructExpression()
{
    auto token = Expect(TokenType_Other, "{");

    std::map<std::string, ExpressionPtr> values;
    while (!At(TokenType_Other, "}"))
    {
        auto name = Expect(TokenType_Symbol).Value;
        Assert(!values.contains(name), "struct expression already has field '{}'", name);

        values[name] = SkipIf(TokenType_Other, ":")
                           ? ParseExpression()
                           : std::make_unique<SymbolExpression>(std::move(token.Loc), name);

        if (!At(TokenType_Other, "}"))
            Expect(TokenType_Other, ",");
    }
    Expect(TokenType_Other, "}");

    TypePtr type;
    if (SkipIf(TokenType_Other, ":"))
        type = ParseType();

    return std::make_unique<StructExpression>(std::move(token.Loc), std::move(values), std::move(type));
}
