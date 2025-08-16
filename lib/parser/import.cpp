#include <fstream>
#include <iostream>
#include <llove/error.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

void llove::Parser::ParseImport()
{
    Expect(TokenType_Symbol, "import");

    bool all;
    std::string as;
    std::map<std::string, std::string> symbols;

    if (SkipIf(TokenType_Operator, "*"))
    {
        all = true;
    }
    else if (At(TokenType_Symbol))
    {
        all = true;
        as = Skip().Value;
    }
    else
    {
        all = false;

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

    Expect(TokenType_Symbol, "from");
    auto filename = Expect(TokenType_String).Value;

    Expect(TokenType_Other, ";");

    std::filesystem::path filepath(filename);
    if (filepath.is_relative())
        filepath = m_Loc.Filepath.parent_path() / filepath;

    std::ifstream stream(filepath);
    Assert(stream.is_open(), "failed to open import file '{}'", filename);

    Parser parser(m_Context, stream, filepath);

    while (parser.Ok())
        if (auto ptr = parser.Parse())
        {
            std::cerr << "import: " << ptr << std::endl;
        }

    stream.close();
}
