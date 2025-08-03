#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseGlobal()
{
    if (SkipIf(TokenType_Symbol, "type"))
    {
        const auto name = Expect(TokenType_Symbol).Value;
        Expect(TokenType_Operator, "=");
        auto type = ParseType();
        m_Types.Set(name, std::move(type));
        return nullptr;
    }

    if (At(TokenType_Symbol, "define", "interface"))
        return ParseDefinitionGlobal();
    if (At(TokenType_Symbol, "class"))
        return ParseClassGlobal();

    Error(m_Token.Loc, "unable to parse global from {} : '{}'", m_Token.Type, m_Token.Value);
}
