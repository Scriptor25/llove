#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseNullExpression()
{
    auto token = Expect(TokenType_Symbol, "null");

    PointerType::Ptr type;
    if (SkipIf(TokenType_Other, ":"))
        type = m_Context.GetPointer(ParseType(), false);

    return std::make_unique<NullExpression>(std::move(token.Loc), std::move(type));
}
