#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::MemberExpression::MemberExpression(
    Location loc,
    ExpressionPtr value,
    std::string member,
    const bool dereference)
    : Expression(std::move(loc)),
      m_Value(std::move(value)),
      m_Member(std::move(member)),
      m_Dereference(dereference)
{
}

llove::ValuePtr llove::MemberExpression::GenVal(
    Builder& builder,
    TypePtr /* expect */) const
try
{
    auto value = m_Value->GenVal(builder, nullptr);

    if (m_Dereference)
    {
        const auto operator_ = builder.FindOperator("*", value->AsField(), false);
        Assert(operator_ != nullptr, "operator '*{}' not implemented", value->AsField());
        value = (*operator_)(builder, std::move(value));
    }

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
        if (!class_type->HasMember(m_Member))
            break;

        Assert(class_type == builder.GetClass(), "field '{}' in type '{}' is not accessible from current context", m_Member, class_type);

        index = class_type->GetMemberIndex(m_Member);
        element = class_type->GetMember(index);
        break;
    }
    default:
        break;
    }

    Assert(element.has_value(), "no field '{}' in type '{}'", m_Member, type);

    builder.EmitLoc(m_Loc);

    if (value->IsReference())
    {
        auto pointer = builder.CreateStructGEP(type->GenIR(builder), value->GetPointer(), index);

        if (element->IsReference())
        {
            pointer = builder.CreateLoad(builder.GetPointerType(), pointer);
            return Value::CreateL(element->GetType(), pointer, element->IsMutable());
        }

        return Value::CreateL(
            element->GetType(),
            pointer,
            value->IsMutable() && element->IsMutable());
    }

    const auto element_value = builder.CreateExtractValue(value->Load(builder), index);

    if (element->IsReference())
        return Value::CreateL(element->GetType(), element_value, element->IsMutable());

    return Value::CreateR(element->GetType(), element_value);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::CalleeInfo llove::MemberExpression::GenCallee(Builder& builder) const
try
{
    auto value = m_Value->GenVal(builder, nullptr);

    if (m_Dereference)
    {
        const auto operator_ = builder.FindOperator("*", value->AsField(), false);
        Assert(operator_ != nullptr, "operator '*{}' not implemented", value->AsField());
        value = (*operator_)(builder, std::move(value));
    }

    auto type = value->GetType();

    if (auto candidates = builder.GetFunctions(m_Member, value->AsField());
        !candidates.empty())
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
        if (!class_type->HasMember(m_Member))
            break;

        Assert(class_type == builder.GetClass(), "field '{}' in type '{}' is not accessible from current context", m_Member, class_type);

        index = class_type->GetMemberIndex(m_Member);
        element = class_type->GetMember(index);
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

    const auto element_type = element->GetType();
    Assert(element_type->IsFunction(), "illegal callee member field '{}' in type '{}', type '{}' is not a function type", element_type);

    llvm::Value* element_value;
    if (value->IsReference())
    {
        auto pointer = builder.CreateStructGEP(type->GenIR(builder), value->GetPointer(), index);

        if (element->IsReference())
            pointer = builder.CreateLoad(builder.GetPointerType(), pointer);

        element_value = builder.CreateLoad(element_type->GenIR(builder), pointer);
    }
    else
    {
        element_value = builder.CreateExtractValue(value->Load(builder), index);

        if (element->IsReference())
            element_value = builder.CreateLoad(element_type->GenIR(builder), element_value);
    }

    FunctionReference reference{
        .IsPublic = true,
        .IsImplicit = false,
        .Name = m_Member,
        .Type = As<FunctionType>(element_type),
        .Callee = element_value,
    };

    return { { std::move(reference) }, {} };
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::MemberExpression::Reflect(Context& context) const
try
{
    ExpressionPtr value;
    if (m_Value)
        m_Value->Reflect(context, value);

    return std::make_unique<MemberExpression>(m_Loc, std::move(value), m_Member, m_Dereference);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream& llove::MemberExpression::Print(std::ostream& stream) const
{
    return stream << m_Value << (m_Dereference ? "::" : ".") << m_Member;
}
