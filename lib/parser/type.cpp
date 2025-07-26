#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseType()
{
    return ParseArrayType();
}

llove::TypePtr llove::Parser::ParseArrayType()
{
    auto base = ParseBaseType();
    while (SkipIf(TokenType_Other, "["))
    {
        if (At(TokenType_Integer))
        {
            const auto size = Skip().IntValue;
            base = m_Types.GetArray(std::move(base), size);
        }
        else
        {
            const auto mutable_ = SkipIf(TokenType_Symbol, "mut");
            base = m_Types.GetPointer(std::move(base), mutable_);
        }
        Expect(TokenType_Other, "]");
    }
    return base;
}

llove::TypePtr llove::Parser::ParseBaseType()
{
    if (SkipIf(TokenType_Other, "{"))
    {
        std::vector<Parameter> fields;

        while (!At(TokenType_Other, "}"))
        {
            Field field;
            auto name = ParseField(field, true);

            fields.emplace_back(field, name);

            if (!At(TokenType_Other, "}"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, "}");

        return m_Types.GetStruct(std::move(fields));
    }

    if (SkipIf(TokenType_Other, "["))
    {
        const auto mutable_ = SkipIf(TokenType_Symbol, "mut");
        Expect(TokenType_Other, "]");
        return m_Types.GetPointer(mutable_);
    }

    if (SkipIf(TokenType_Other, "("))
    {
        std::vector<Field> parameters;
        auto vararg = false;

        while (!At(TokenType_Other, ")"))
        {
            if (SkipIf(TokenType_Operator, "..."))
            {
                vararg = true;
                break;
            }

            ParseField(parameters.emplace_back(), false, false);

            if (!At(TokenType_Other, ")"))
                Expect(TokenType_Other, ",");
        }
        Expect(TokenType_Other, ")");

        auto has_self = false;
        Field self;
        if (SkipIf(TokenType_Other, "["))
        {
            has_self = true;
            ParseField(self, false, false);
            Expect(TokenType_Other, "]");
        }

        Field result;
        if (SkipIf(TokenType_Operator, "=>"))
            ParseField(result, false, false);

        if (has_self)
            return m_Types.GetFunction(std::move(parameters), vararg, std::move(result), std::move(self));
        return m_Types.GetFunction(std::move(parameters), vararg, std::move(result));
    }

    if (SkipIf(TokenType_Symbol, "class"))
    {
        if (SkipIf(TokenType_Operator, "<"))
        {
            std::vector<TypePtr> template_arguments;
            while (!At(TokenType_Operator, ">"))
            {
                template_arguments.emplace_back(ParseType());

                if (!At(TokenType_Operator, ">"))
                    Expect(TokenType_Other, ",");
            }
            Expect(TokenType_Operator, ">");

            auto name = Expect(TokenType_Symbol).Value;
            return m_Types.InstantiateTemplateClass(m_Builder, std::move(name), template_arguments);
        }

        auto name = Expect(TokenType_Symbol).Value;
        return m_Types.GetClass(std::move(name));
    }

    if (SkipIf(TokenType_Symbol, "range"))
    {
        Expect(TokenType_Operator, "<");
        auto entry = ParseType();
        Expect(TokenType_Operator, ">");

        return m_Types.GetRange(std::move(entry));
    }

    if (At(TokenType_Symbol))
    {
        const auto name = Expect(TokenType_Symbol).Value;
        if (auto type = m_Types.Get(name))
            return type;

        if (name == "void")
            return m_Types.GetVoid();
        if (name == "i1")
            return m_Types.GetInteger(true, 1);
        if (name == "i8")
            return m_Types.GetInteger(true, 8);
        if (name == "i16")
            return m_Types.GetInteger(true, 16);
        if (name == "i32")
            return m_Types.GetInteger(true, 32);
        if (name == "i64")
            return m_Types.GetInteger(true, 64);
        if (name == "u1")
            return m_Types.GetInteger(false, 1);
        if (name == "u8")
            return m_Types.GetInteger(false, 8);
        if (name == "u16")
            return m_Types.GetInteger(false, 16);
        if (name == "u32")
            return m_Types.GetInteger(false, 32);
        if (name == "u64")
            return m_Types.GetInteger(false, 64);
        if (name == "f16")
            return m_Types.GetFloat(16);
        if (name == "f32")
            return m_Types.GetFloat(32);
        if (name == "f64")
            return m_Types.GetFloat(64);

        Error("undefined type '{}'", name);
    }

    Error("unable to parse type from {} : '{}'", m_Token.Type, m_Token.Value);
}
