#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseGlobal()
{
    if (SkipIf(TokenType_Sym, "type"))
    {
        const auto name = Expect(TokenType_Sym).Value;
        Expect(TokenType_Opr, "=");
        auto type = ParseType();
        m_Types.Set(name, std::move(type));
        return nullptr;
    }

    if (At(TokenType_Sym, "define", "interface"))
        return ParseDefinitionGlobal();
    if (At(TokenType_Sym, "class"))
        return ParseClassGlobal();

    Error("unable to parse global from {} : '{}'", m_Token.Type, m_Token.Value);
}
