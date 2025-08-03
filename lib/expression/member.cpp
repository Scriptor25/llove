#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::MemberExpression::MemberExpression(Location loc, ExpressionPtr value, std::string member)
    : Expression(std::move(loc)),
      m_Value(std::move(value)),
      m_Member(std::move(member))
{
}

llove::ValuePtr llove::MemberExpression::GenVal(Builder &builder, TypePtr expect) const try
{
    const auto value = m_Value->GenVal(builder, nullptr);
    const auto type = value->GetType();

    auto index = ~0u;
    std::optional<Field> element;

    switch (type->GetId())
    {
    case TypeId_Struct:
    {
        const auto struct_type = As<StructType>(type);
        if (!struct_type->HasField(m_Member))
            break;

        index = struct_type->GetFieldIndex(m_Member);
        element = struct_type->GetField(index);
        break;
    }
    case TypeId_Class:
    {
        const auto class_type = As<ClassType>(type);
        if (!class_type->HasField(m_Member))
            break;

        Assert(
            class_type == builder.GetClass(),
            "field '{}' in type '{}' is not accessible from current context",
            m_Member,
            class_type);

        index = class_type->GetFieldIndex(m_Member);
        element = class_type->GetField(index);
        break;
    }
    default:
        break;
    }

    Assert(element.has_value(), "no field '{}' in type '{}'", m_Member, type);

    builder.EmitLoc(m_Loc);

    if (value->IsReferenceable())
    {
        auto pointer = builder.CreateStructGEP(type, value->GetPointer(), index);

        if (element->Reference)
        {
            const auto pointer_type = builder.GetTypes().GetPointer(element->Type, element->Mutable);
            pointer = builder.CreateLoad(pointer, pointer_type);
            return Value::CreateL(element->Type, pointer, element->Mutable);
        }

        return Value::CreateL(element->Type, pointer, value->IsMutable() && element->Mutable);
    }

    const auto element_value = builder.CreateExtractValue(value->Load(builder), index);

    if (element->Reference)
        return Value::CreateL(element->Type, element_value, element->Mutable);

    return Value::CreateR(element->Type, element_value);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::CalleeInfo llove::MemberExpression::GenCallee(Builder &builder) const try
{
    auto value = m_Value->GenVal(builder, nullptr);
    auto type = value->GetType();

    if (auto candidates = builder.GetFunctions(m_Member, value->AsField()); !candidates.empty())
        return { .Candidates = std::move(candidates), .Self = std::move(value) };

    auto index = ~0u;
    std::optional<Field> element;

    switch (type->GetId())
    {
    case TypeId_Struct:
    {
        const auto struct_type = As<StructType>(type);
        if (!struct_type->HasField(m_Member))
            break;

        index = struct_type->GetFieldIndex(m_Member);
        element = struct_type->GetField(index);
        break;
    }
    case TypeId_Class:
    {
        const auto class_type = As<ClassType>(type);
        if (!class_type->HasField(m_Member))
            break;

        Assert(
            class_type == builder.GetClass(),
            "field '{}' in type '{}' is not accessible from current context",
            m_Member,
            class_type);

        index = class_type->GetFieldIndex(m_Member);
        element = class_type->GetField(index);
        break;
    }
    default:
        break;
    }

    Assert(
        element.has_value(),
        "no field '{}' in type '{}' or function with self '{}'",
        m_Member,
        type,
        value->AsField());

    const auto element_type = element->Type;
    Assert(
        element_type->IsFunction(),
        "illegal callee member field '{}' in type '{}', type '{}' is not a function type",
        element_type);

    llvm::Value *element_value;
    if (value->IsReferenceable())
    {
        auto pointer = builder.CreateStructGEP(type, value->GetPointer(), index);

        if (element->Reference)
        {
            const auto pointer_type = builder.GetTypes().GetPointer(element->Type, element->Mutable);
            pointer = builder.CreateLoad(pointer, pointer_type);
        }

        element_value = builder.CreateLoad(pointer, element_type);
    }
    else
    {
        element_value = builder.CreateExtractValue(value->Load(builder), index);

        if (element->Reference)
            element_value = builder.CreateLoad(element_value, element->Type);
    }

    FunctionReference reference
    {
        .Expose = true,
        .Implicit = false,
        .Name = m_Member,
        .Type = As<FunctionType>(element_type),
        .Callee = element_value,
    };

    return { .Candidates = { std::move(reference) } };
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::MemberExpression::Reflect(Builder &builder) const try
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(builder, value);

    return std::make_unique<MemberExpression>(m_Loc, std::move(value), m_Member);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::MemberExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '.' << m_Member;
}
