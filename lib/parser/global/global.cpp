#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseGlobal(bool is_template)
try
{
    const auto is_export = !is_template && SkipIf(TokenType_Symbol, "export");

    if (!is_template && !is_export && At(TokenType_Symbol, "import"))
        return ParseImportGlobal();

    if (!is_template && At(TokenType_Symbol, "const"))
        return ParseConstGlobal(is_export);
    if (!is_template && At(TokenType_Symbol, "let"))
        return ParseLetGlobal(is_export);
    if (!is_template && At(TokenType_Symbol, "template"))
        return ParseTemplateGlobal(is_export);

    if (At(TokenType_Symbol, "class"))
        return ParseClassGlobal(is_template, is_export);
    if (At(TokenType_Symbol, "function", "interface"))
        return ParseFunctionGlobal(is_template, is_export);
    if (At(TokenType_Symbol, "type"))
        return ParseTypeGlobal(is_template, is_export);

    Error("unable to parse global from {} : '{}'", m_Token.Type, m_Token.Value);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Token.Loc, std::nullopt);
}
