#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseBaseType()
{
    if (At(TokenType_Symbol, "class"))
        return ParseClassType();

    if (At(TokenType_Other, "("))
        return ParseFunctionType();

    if (At(TokenType_Other, "["))
        return ParsePointerType();

    if (At(TokenType_Symbol, "range"))
        return ParseRangeType();

    if (At(TokenType_Other, "{"))
        return ParseStructType();

    if (At(TokenType_Symbol))
        return ParseNamedType();

    Error(m_Token.Loc, "unable to parse type from {} : '{}'", m_Token.Type, m_Token.Value);
}
