#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::LetStatement::LetStatement(
    Location loc,
    Field field,
    std::string name,
    ExpressionPtr value,
    std::vector<ExpressionPtr> arguments)
    : Statement(std::move(loc)),
      m_Field(std::move(field)),
      m_Name(std::move(name)),
      m_Value(std::move(value)),
      m_Arguments(std::move(arguments))
{
}

void llove::LetStatement::Gen(Builder& builder) const
try
{
    ValuePtr value;
    TypePtr type;

    if (m_Field.HasType())
    {
        type = m_Field.GetType();
        if (m_Value)
        {
            value = m_Value->GenVal(builder, type);
        }
    }
    else
    {
        value = m_Value->GenVal(builder, nullptr);
        type = value->GetType();
    }

    std::vector<Field> argument_fields;
    std::vector<ValuePtr> argument_values;
    for (auto& argument : m_Arguments)
    {
        auto argument_value = argument->GenVal(builder, nullptr);
        argument_fields.emplace_back(argument_value->AsField());
        argument_values.emplace_back(std::move(argument_value));
    }

    builder.EmitLoc(m_Loc);

    llvm::Value* pointer;
    if (m_Field.IsReference())
    {
        Assert(m_Arguments.empty(), "cannot construct reference");
        Assert(value != nullptr, "missing initializer value");
        Assert(value->IsReference(), "reference from rvalue");
        Assert(type == value->GetType(), "reference type mismatch");
        Assert(!m_Field.IsMutable() || value->IsMutable(), "reference mutability violation");

        pointer = value->GetPointer();
    }
    else
    {
        pointer = builder.CreateAlloca(type->GenIR(builder));

        if (type->IsClass())
        {
            const auto self = Value::CreateL(type, pointer, true);

            const auto class_type = As<ClassType>(type);
            const auto constructors = class_type->GetConstructors(class_type);

            if (value)
            {
                if (const auto
                        candidate = builder.FindFunction(constructors, { value->AsField() }, self->AsField(), true))
                {
                    builder.CreateCall(*candidate, { std::move(value) }, self);
                }
                else
                {
                    Assert(!value->IsReference(), "illegal implicit copy");
                    value = builder.CreateCast(std::move(value), type, true);
                    builder.CreateStore(value->Load(builder), pointer);
                }
            }
            else if (constructors.empty())
            {
                Assert(argument_values.empty(), "illegal arguments for implicit default constructor");

                builder.CreateStore(llvm::Constant::getNullValue(type->GenIR(builder)), pointer);
            }
            else
            {
                const auto candidate = builder.FindFunction(constructors, argument_fields, self->AsField(), false);
                Assert(candidate.has_value(), "no suitable candidate");

                builder.CreateCall(*candidate, std::move(argument_values), self);
            }

            builder.PushDestructor(pointer, class_type);
        }
        else
        {
            if (!value)
            {
                Assert(argument_values.empty(), "cannot construct non-class value");

                value = Value::CreateR(type, llvm::Constant::getNullValue(type->GenIR(builder)));
            }
            else if (type)
            {
                value = builder.CreateCast(std::move(value), type, true);
            }

            builder.CreateStore(value->Load(builder), pointer);
        }
    }

    auto storage = Value::CreateL(std::move(type), pointer, m_Field.IsMutable());
    builder.GetDebug().CreateVariable(builder, m_Name, storage);
    builder.SetValue(m_Name, std::move(storage));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::LetStatement::Reflect(Context& context) const
try
{
    Field field;
    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments;

    m_Field.Reflect(context, field);

    if (m_Value)
        m_Value->Reflect(context, value);

    for (auto& argument : m_Arguments)
        argument->Reflect(context, arguments.emplace_back());

    return std::make_unique<LetStatement>(m_Loc, std::move(field), m_Name, std::move(value), std::move(arguments));
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream& llove::LetStatement::Print(std::ostream& stream) const
{
    m_Field.Print(stream << "let ", true, m_Name);

    if (m_Value)
        return stream << " = " << m_Value << ';';

    if (m_Arguments.empty())
        return stream << ';';

    stream << '(';
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << ");";
}
