#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseGlobal()
try
{
    if (At(TokenType_Symbol, "import"))
        return ParseImportGlobal();

    const auto export_ = SkipIf(TokenType_Symbol, "export");

    if (At(TokenType_Symbol, "type"))
        return ParseTypeGlobal(export_);

    if (At(TokenType_Symbol, "define", "interface"))
        return ParseDefinitionGlobal(export_);
    if (At(TokenType_Symbol, "class"))
        return ParseClassGlobal(export_);
    if (At(TokenType_Symbol, "const"))
        return ParseConstGlobal(export_);

    Error("unable to parse global from {} : '{}'", m_Token.Type, m_Token.Value);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Token.Loc, std::nullopt);
}
