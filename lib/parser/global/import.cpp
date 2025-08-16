#include <llove/parser.hpp>
#include <llove/tree.hpp>

llove::GlobalPtr llove::Parser::ParseImportGlobal()
{
    auto loc = Expect(TokenType_Symbol, "import").Loc;

    std::string as;
    std::map<std::string, std::string> symbols;

    if (!SkipIf(TokenType_Operator, "*"))
    {
        if (At(TokenType_Symbol))
        {
            as = Skip().Value;
        }
        else
        {
            Expect(TokenType_Other, "{");
            while (!At(TokenType_Other, "}"))
            {
                if (SkipIf(TokenType_Operator, "..."))
                {
                    as = Expect(TokenType_Symbol).Value;
                    break;
                }

                auto name = Expect(TokenType_Symbol).Value;
                if (SkipIf(TokenType_Other, ":"))
                {
                    auto remap = Expect(TokenType_Symbol).Value;
                    symbols[name] = std::move(remap);
                }
                else
                {
                    symbols[name] = name;
                }

                if (!At(TokenType_Other, "}"))
                    Expect(TokenType_Other, ",");
            }
            Expect(TokenType_Other, "}");
        }
    }

    Expect(TokenType_Symbol, "from");
    auto filename = Expect(TokenType_String).Value;

    Expect(TokenType_Other, ";");

    std::filesystem::path filepath(std::move(filename));
    if (filepath.is_relative())
        filepath = m_Loc.Filepath.parent_path() / filepath;

    return std::make_unique<ImportGlobal>(std::move(loc), std::move(as), std::move(symbols), std::move(filepath));
}
