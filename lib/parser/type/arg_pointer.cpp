#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseArgPointerType()
{
    Expect(TokenType_Operator, "<...>");

    return m_Context.GetPointer(false);
}
