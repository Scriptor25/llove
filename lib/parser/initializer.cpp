#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseInitializer(Initializer& initializer)
{
    auto token = Expect(TokenType_Symbol);
    initializer.Name = std::move(token.Value);

    if (initializer.Name == "create")
    {
        ParseArgumentList(initializer.Arguments);
        return;
    }

    if (SkipIf(TokenType_Operator, "="))
    {
        initializer.Value = ParseExpression();
        return;
    }

    if (At(TokenType_Other, "("))
    {
        ParseArgumentList(initializer.Arguments);
        return;
    }

    initializer.Value = std::make_unique<SymbolExpression>(
        std::move(token.Loc),
        initializer.Name);
}
