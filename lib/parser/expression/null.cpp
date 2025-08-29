#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::ExpressionPtr llove::Parser::ParseNullExpression()
{
    auto token = Expect(TokenType_Symbol, "null");

    PointerType::Ptr type;
    if (SkipIf(TokenType_Operator, ":"))
    {
        const auto is_mutable = SkipIf(TokenType_Symbol, "mut");

        TypePtr base;
        if (!is_mutable || SkipIf(TokenType_Operator, ":"))
            type = m_Context.GetPointer(ParseType(), is_mutable);
        else
            type = m_Context.GetPointer(is_mutable);
    }

    return std::make_unique<NullExpression>(std::move(token.Loc), std::move(type));
}
