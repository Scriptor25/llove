#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::MemberExpression::MemberExpression(ExpressionPtr value, std::string member)
    : m_Value(std::move(value)),
      m_Member(std::move(member))
{
}

llove::ValuePtr llove::MemberExpression::GenVal(Builder &builder, TypePtr expect) const
{
    const auto value = m_Value->GenVal(builder, nullptr);
    const auto type = value->GetType();

    unsigned index;
    Field element;

    if (const auto struct_type = As<StructType>(type))
    {
        index = struct_type->GetFieldIndex(m_Member);
        element = struct_type->GetField(index);
    }
    else if (const auto class_type = As<ClassType>(type))
    {
        // TODO: check if field is accessible
        // TODO: if no field with name exists, return single function with name if exists and is accessible
        index = class_type->GetFieldIndex(m_Member);
        element = class_type->GetField(index);
    }
    else
    {
        Error("not yet implemented");
    }

    if (value->IsReferenceable())
    {
        const auto result = builder.CreateStructGEP(type, value->GetPointer(), index);

        if (element.Reference)
        {
            const auto pointer = builder.CreateLoad(result, builder.GetTypes().GetPointer(false));
            return Value::CreateL(element.Type, pointer, element.Mutable);
        }

        return Value::CreateL(element.Type, result, value->IsMutable() && element.Mutable);
    }

    const auto result = builder.CreateExtractValue(value->Load(builder), index);

    if (element.Reference)
    {
        const auto pointer = builder.CreateLoad(result, builder.GetTypes().GetPointer(false));
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

std::ostream &llove::MemberExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '.' << m_Member;
}
