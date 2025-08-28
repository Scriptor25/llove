#include <llove/builder.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::CreateExpression::CreateExpression(
    Location loc,
    TypePtr type,
    ExpressionPtr destination,
    std::vector<ExpressionPtr> arguments)
    : Expression(std::move(loc)),
      m_Type(std::move(type)),
      m_Destination(std::move(destination)),
      m_Arguments(std::move(arguments))
{
}

llove::ValuePtr llove::CreateExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    Assert(m_Type->IsClass(), "cannot construct non-class value");

    const auto class_type = As<ClassType>(m_Type);
    auto destination = m_Destination ? m_Destination->GenVal(builder, m_Type) : nullptr;

    const Field self(true, true, m_Type);

    const auto constructors = class_type->GetConstructors(class_type);

    std::vector<Field> argument_fields;
    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
    {
        auto argument_value = argument->GenVal(builder, nullptr);
        argument_fields.emplace_back(argument_value->AsField());
        arguments.emplace_back(std::move(argument_value));
    }

    const auto candidate = builder.FindFunction(constructors, argument_fields, self, false);
    Assert(candidate.has_value(), "no suitable candidate");

    llvm::Value *pointer;
    if (destination)
    {
        Assert(destination->GetType() == m_Type, "destination type mismatch");
        Assert(destination->IsReference(), "destination is rvalue");
        Assert(destination->IsMutable(), "destination mutability violation");
        pointer = destination->GetPointer();
    }
    else
    {
        pointer = builder.CreateAlloca(m_Type->GenIR(builder));
    }

    builder.EmitLoc(m_Loc);

    builder.CreateCall(*candidate, std::move(arguments), Value::CreateL(class_type, pointer, true));

    if (destination)
        return destination;

    const auto value = builder.CreateLoad(m_Type->GenIR(builder), pointer);
    return Value::CreateR(m_Type, value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::CreateExpression::Reflect(Context &context) const try
{
    TypePtr type;
    Type::Reflect(context, m_Type, type);

    ExpressionPtr destination;
    if (m_Destination)
        m_Destination->Reflect(context, destination);

    std::vector<ExpressionPtr> arguments;
    for (auto &argument : m_Arguments)
        argument->Reflect(context, arguments.emplace_back());

    return std::make_unique<CreateExpression>(m_Loc, std::move(type), std::move(destination), std::move(arguments));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::CreateExpression::Print(std::ostream &stream) const
{
    stream << "create";
    if (m_Destination)
        stream << '[' << m_Destination << ']';
    stream << ' ' << m_Type;
    if (m_Arguments.empty())
        return stream;

    stream << '(';
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << ')';
}
