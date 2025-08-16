#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseGlobal()
{
    if (At(TokenType_Symbol, "type"))
    {
        ParseTypeAlias();
        return nullptr;
    }

    if (At(TokenType_Symbol, "import"))
        return ParseImportGlobal();

    const auto export_ = SkipIf(TokenType_Symbol, "export");

    if (At(TokenType_Symbol, "define", "interface"))
        return ParseDefinitionGlobal(export_);
    if (At(TokenType_Symbol, "class"))
        return ParseClassGlobal(export_);
    if (At(TokenType_Symbol, "const"))
        return ParseConstGlobal(export_);

    Error(m_Token.Loc, "unable to parse global from {} : '{}'", m_Token.Type, m_Token.Value);
}
