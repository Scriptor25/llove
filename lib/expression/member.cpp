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

llove::ValuePtr llove::MemberExpression::GenVal(Builder &builder, TypePtr expect) const
{
    const auto value = m_Value->GenVal(builder, nullptr);
    const auto type = value->GetType();

    unsigned index;
    Field element;

    switch (type->GetId())
    {
    case TypeId_Struct:
    {
        const auto struct_type = As<StructType>(type);
        Assert(struct_type->HasField(m_Member), "no field '{}' in type {}", m_Member, struct_type);
        index = struct_type->GetFieldIndex(m_Member);
        element = struct_type->GetField(index);
        break;
    }
    case TypeId_Class:
    {
        const auto class_type = As<ClassType>(type);
        Assert(class_type->HasField(m_Member), "no field '{}' in type {}", m_Member, class_type);
        Assert(
            class_type == builder.GetClass(),
            "field '{}' in type {} is not accessible from current context",
            m_Member,
            class_type);
        // TODO: if no field with name exists, return single function with name if exists and is accessible
        index = class_type->GetFieldIndex(m_Member);
        element = class_type->GetField(index);
        break;
    }
    default:
        Error("not implemented");
    }

    builder.EmitLoc(m_Loc);

    if (value->IsReferenceable())
    {
        auto pointer = builder.CreateStructGEP(type, value->GetPointer(), index);

        if (element.Reference)
        {
            pointer = builder.CreateLoad(pointer, builder.GetTypes().GetPointer(element.Type, element.Mutable));
            return Value::CreateL(element.Type, pointer, element.Mutable);
        }

        return Value::CreateL(element.Type, pointer, value->IsMutable() && element.Mutable);
    }

    const auto result = builder.CreateExtractValue(value->Load(builder), index);

    if (element.Reference)
    {
        const auto pointer = builder.CreateLoad(result, builder.GetTypes().GetPointer(element.Type, element.Mutable));
        return Value::CreateL(element.Type, pointer, element.Mutable);
    }

    return Value::CreateR(element.Type, result);
}

llove::CalleeInfo llove::MemberExpression::GenCallee(Builder &builder) const
{
    // TODO: if field with name exists and is accessible, add to candidates

    auto value = m_Value->GenVal(builder, nullptr);
    return { .Candidates = builder.GetFunctions(m_Member, value->AsField()), .Self = std::move(value) };
}

llove::StatementPtr llove::MemberExpression::Reflect(Builder &builder) const
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(builder, value);

    return std::make_unique<MemberExpression>(m_Loc, std::move(value), m_Member);
}

std::ostream &llove::MemberExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '.' << m_Member;
}
