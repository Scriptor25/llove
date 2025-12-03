#include <llove/forward.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>
#include <utility>
#include <vector>

llove::ExpressionPtr llove::Parser::ParseInlineExpression()
{
    auto loc = Expect(TokenType_Symbol, "inline").Loc;

    Expect(TokenType_Other, "(");

    auto asm_string = Expect(TokenType_String).Value;

    bool sideeffect = false, alignstack = false, inteldialect = false, unwind = false;

    if (SkipIf(TokenType_Other, "["))
    {
        while (!At(TokenType_Other, "]"))
        {
            auto symbol = Expect(TokenType_Symbol, "sideeffect", "alignstack", "inteldialect", "unwind");
            auto& flag = symbol.Value;
            if (flag == "sideeffect")
                sideeffect = true;
            else if (flag == "alignstack")
                alignstack = true;
            else if (flag == "inteldialect")
                inteldialect = true;
            else if (flag == "unwind")
                unwind = true;
            else
                Error(symbol.Loc, "undefined inline flag '{}'", flag);
        }
        Expect(TokenType_Other, "]");
    }

    std::vector<InlineOperand> dst_operands, src_operands;
    std::vector<std::string> clobbers;

    if (SkipIf(TokenType_Operator, "|"))
    {
        while (At(TokenType_String))
        {
            auto constraint = Skip().Value;
            Expect(TokenType_Operator, ":");
            auto type = ParseType();

            dst_operands.emplace_back(std::move(constraint), std::move(type));

            if (!SkipIf(TokenType_Other, ","))
                break;
        }
    }

    if (SkipIf(TokenType_Operator, "|"))
    {
        while (At(TokenType_String))
        {
            auto constraint = Skip().Value;
            Expect(TokenType_Operator, ":");
            auto type = ParseType();

            src_operands.emplace_back(std::move(constraint), std::move(type));

            if (!SkipIf(TokenType_Other, ","))
                break;
        }
    }

    if (SkipIf(TokenType_Operator, "|"))
    {
        while (At(TokenType_String))
        {
            auto constraint = Skip().Value;

            clobbers.emplace_back(std::move(constraint));

            if (!SkipIf(TokenType_Other, ","))
                break;
        }
    }

    Expect(TokenType_Other, ")");

    return std::make_unique<InlineExpression>(std::move(loc), std::move(asm_string), std::move(dst_operands), std::move(src_operands), std::move(clobbers), sideeffect, alignstack, inteldialect, unwind);
}
