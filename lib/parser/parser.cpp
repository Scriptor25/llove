#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::Parser::Parser(Context &context, std::istream &stream, const std::filesystem::path &filepath)
    : m_Context(context),
      m_Stream(stream),
      m_Buffer(0),
      m_Loc(filepath, 1u, 0u)
{
    Get();
    m_Token = Next();
}

bool llove::Parser::Ok() const
{
    return m_Token.Type != TokenType_EndOfFile;
}

llove::GlobalPtr llove::Parser::Parse()
{
    return ParseGlobal();
}
