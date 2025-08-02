#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParsePointerType()
{
    Expect(TokenType_Other, "[");
    const auto mutable_ = SkipIf(TokenType_Symbol, "mut");
    Expect(TokenType_Other, "]");
    return m_Types.GetPointer(mutable_);
}
