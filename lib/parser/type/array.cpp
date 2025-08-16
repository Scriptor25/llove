#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseArrayType()
{
    auto base = ParseBaseType();
    while (SkipIf(TokenType_Other, "["))
    {
        if (At(TokenType_Integer))
        {
            const auto size = Skip().IntegerValue;
            base = m_Context.GetArray(std::move(base), size);
        }
        else
        {
            const auto mutable_ = SkipIf(TokenType_Symbol, "mut");
            base = m_Context.GetPointer(std::move(base), mutable_);
        }
        Expect(TokenType_Other, "]");
    }
    return base;
}
