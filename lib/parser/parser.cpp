#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::Parser::Parser(Context &types, Builder &builder, std::istream &stream)
    : m_Types(types),
      m_Builder(builder),
      m_Stream(stream),
      m_Buffer(0),
      m_Loc({}, 1u, 0u)
{
    Get();
    m_Token = Next();
}

bool llove::Parser::Ok() const
{
    return m_Token.Type != TokenType_Eof;
}

llove::GlobalPtr llove::Parser::Parse()
{
    return ParseGlobal();
}
