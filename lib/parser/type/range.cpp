#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseRangeType()
{
    Expect(TokenType_Symbol, "range");

    Expect(TokenType_Operator, "<");
    auto entry = ParseType();
    Expect(TokenType_Operator, ">");

    return m_Types.GetRange(std::move(entry));
}
