#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseClassDefinitionGlobal()
{
    auto class_name = Expect(TokenType_Sym).Value;
    auto class_type = m_Types.GetClass(std::move(class_name));

    auto mutable_ = SkipIf(TokenType_Sym, "mut");
    auto name = At(TokenType_Opr) ? Skip().Value : Expect(TokenType_Sym).Value;

    std::vector<Parameter> parameters;
    auto vararg = false;

    Expect(TokenType_Otr, "(");
    while (!At(TokenType_Otr, ")"))
    {
        if (SkipIf(TokenType_Opr, "..."))
        {
            vararg = true;
            break;
        }

        auto &[info_, name_] = parameters.emplace_back();
        name_ = ParseField(info_);

        if (!At(TokenType_Otr, ")"))
            Expect(TokenType_Otr, ",");
    }
    Expect(TokenType_Otr, ")");

    Field result;
    if (SkipIf(TokenType_Otr, ":"))
        ParseField(result, false, false);
    else
        result.Type = m_Types.GetVoid();

    auto content = ParseScopeStatement();

    return std::make_unique<ClassDefinitionGlobal>(
        std::move(class_type),
        mutable_,
        std::move(name),
        std::move(parameters),
        vararg,
        std::move(result),
        std::move(content));
}
