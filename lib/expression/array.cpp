#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ArrayExpression::ArrayExpression(Location loc, std::vector<ExpressionPtr> values, TypePtr type)
    : Expression(std::move(loc)),
      m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::ValuePtr llove::ArrayExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    auto type = m_Type && (m_Type->IsArray() || m_Type->IsTuple())
                    ? m_Type
                    : expect && (expect->IsArray() || expect->IsTuple())
                    ? std::move(expect)
                    : nullptr;
    Assert(type != nullptr, "untyped array expression");

    auto get_element = [&](const unsigned index)
    {
        return type->IsArray() ? Field(As<ArrayType>(type)->GetBase()) : As<TupleType>(type)->GetField(index);
    };

    builder.EmitLoc(m_Loc);

    llvm::Value *aggregate = llvm::Constant::getNullValue(type->GenIR(builder));

    for (unsigned index = 0; index < m_Values.size(); ++index)
    {
        auto element = get_element(index);

        auto gen_val = m_Values[index]->GenVal(builder, element.GetType());
        const auto val = element.GenCast(builder, std::move(gen_val));

        aggregate = builder.CreateInsertValue(aggregate, val, index);
    }

    return Value::CreateR(std::move(type), aggregate);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::ArrayExpression::Reflect(Context &context) const try
{
    std::vector<ExpressionPtr> values;
    for (auto &value : m_Values)
        value->Reflect(context, values.emplace_back());

    TypePtr type;
    Type::Reflect(context, m_Type, type);

    return std::make_unique<ArrayExpression>(m_Loc, std::move(values), std::move(type));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::ArrayExpression::Print(std::ostream &stream) const
{
    stream << "{ ";
    for (auto i = m_Values.begin(); i != m_Values.end(); ++i)
    {
        if (i != m_Values.begin())
            stream << ", ";
        stream << *i;
    }
    stream << " }";
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}
