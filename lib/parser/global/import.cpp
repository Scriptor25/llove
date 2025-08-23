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

    auto basepath = std::filesystem::weakly_canonical(filename);

    std::filesystem::path filepath;
    if (basepath.is_relative())
    {
        filepath = weakly_canonical(m_Loc.Filepath.parent_path() / basepath);
        for (auto i = m_Includes.begin(); i != m_Includes.end() && !exists(filepath); ++i)
            filepath = weakly_canonical(*i / basepath);
    }
    else
    {
        filepath = std::move(basepath);
    }

    Assert(exists(filepath), "imported file name '{}' ({}) does not exist", filename, filepath.string());

    return std::make_unique<ImportGlobal>(
        std::move(loc),
        std::move(as),
        std::move(symbols),
        std::move(filepath),
        m_Includes);
}
