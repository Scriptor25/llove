#include <llove/context.hpp>
#include <llove/parser.hpp>

llove::TypePtr llove::Parser::ParseType()
{
    return ParseArrayType();
}

llove::TypePtr llove::Parser::ParseArrayType()
{
    auto base = ParseBaseType();
    while (SkipIf(TokenType_Otr, "["))
    {
        if (At(TokenType_Int))
        {
            const auto size = Skip().IntValue;
            base = m_Types.GetArray(std::move(base), size);
        }
        else
        {
            const auto mutable_ = SkipIf(TokenType_Sym, "mut");
            base = m_Types.GetPointer(std::move(base), mutable_);
        }
        Expect(TokenType_Otr, "]");
    }
    return base;
}

llove::TypePtr llove::Parser::ParseBaseType()
{
    if (SkipIf(TokenType_Otr, "{"))
    {
        std::vector<Parameter> fields;

        while (!At(TokenType_Otr, "}"))
        {
            Field field;
            auto name = ParseField(field, true);

            fields.emplace_back(field, name);

            if (!At(TokenType_Otr, "}"))
                Expect(TokenType_Otr, ",");
        }
        Expect(TokenType_Otr, "}");

        return m_Types.GetStruct(std::move(fields));
    }

    if (SkipIf(TokenType_Otr, "["))
    {
        const auto mutable_ = SkipIf(TokenType_Sym, "mut");
        Expect(TokenType_Otr, "]");
        return m_Types.GetPointer(mutable_);
    }

    if (SkipIf(TokenType_Otr, "("))
    {
        std::vector<Field> parameters;
        auto vararg = false;

        while (!At(TokenType_Otr, ")"))
        {
            if (SkipIf(TokenType_Opr, "..."))
            {
                vararg = true;
                break;
            }

            ParseField(parameters.emplace_back(), false, false);

            if (!At(TokenType_Otr, ")"))
                Expect(TokenType_Otr, ",");
        }
        Expect(TokenType_Otr, ")");

        auto has_self = false;
        Field self;
        if (SkipIf(TokenType_Otr, "["))
        {
            has_self = true;
            ParseField(self, false, false);
            Expect(TokenType_Otr, "]");
        }

        Field result;
        if (SkipIf(TokenType_Opr, "=>"))
            ParseField(result, false, false);

        if (has_self)
            return m_Types.GetFunction(std::move(parameters), vararg, std::move(result), std::move(self));
        return m_Types.GetFunction(std::move(parameters), vararg, std::move(result));
    }

    if (SkipIf(TokenType_Sym, "class"))
    {
        if (SkipIf(TokenType_Opr, "<"))
        {
            std::vector<TypePtr> template_arguments;
            while (!At(TokenType_Opr, ">"))
            {
                template_arguments.emplace_back(ParseType());

                if (!At(TokenType_Opr, ">"))
                    Expect(TokenType_Otr, ",");
            }
            Expect(TokenType_Opr, ">");

            auto name = Expect(TokenType_Sym).Value;
            return m_Types.InstantiateTemplateClass(m_Builder, std::move(name), template_arguments);
        }

        auto name = Expect(TokenType_Sym).Value;
        return m_Types.GetClass(std::move(name));
    }

    if (SkipIf(TokenType_Sym, "range"))
    {
        Expect(TokenType_Opr, "<");
        auto entry = ParseType();
        Expect(TokenType_Opr, ">");

        return m_Types.GetRange(std::move(entry));
    }

    if (At(TokenType_Sym))
    {
        const auto name = Expect(TokenType_Sym).Value;
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
