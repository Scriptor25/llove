#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::LetStatement::LetStatement(
    Field info,
    std::string name,
    ExpressionPtr value,
    std::vector<ExpressionPtr> arguments)
    : m_Info(std::move(info)),
      m_Name(std::move(name)),
      m_Value(std::move(value)),
      m_Arguments(std::move(arguments))
{
}

void llove::LetStatement::Gen(Builder &builder) const
{
    Assert(m_Info.Type != nullptr || m_Value != nullptr, "missing type or value");

    auto value = m_Value ? m_Value->GenVal(builder, m_Info.Type) : nullptr;
    auto type = m_Info.Type ? m_Info.Type : value->GetType();

    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
        arguments.emplace_back(argument->GenVal(builder, nullptr));

    ValuePtr storage;
    if (m_Info.Reference)
    {
        Assert(arguments.empty(), "cannot construct reference");
        Assert(value != nullptr, "missing initializer value");
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(type == value->GetType(), "reference type mismatch");
        Assert(!m_Info.Mutable || value->IsMutable(), "reference mutability violation");

        storage = Value::CreateL(std::move(type), value->GetPointer(), m_Info.Mutable);
    }
    else
    {
        const auto pointer = builder.CreateAlloca(type);

        if (type && type->IsClass())
        {
            const Field self
            {
                .Mutable = true,
                .Reference = true,
                .Type = type,
            };

            const auto class_type = As<ClassType>(type);
            const auto constructors = class_type->GetConstructors();

            if (arguments.empty())
            {
                std::vector<Field> argument_fields;
                std::vector<ValuePtr> argument_values;
                if (value)
                {
                    argument_fields.emplace_back(value->AsField());
                    argument_values.emplace_back(value);
                }

                if (const auto candidate = builder.FindFunction(constructors, argument_fields, class_type, self))
                {
                    builder.CreateCall(
                        candidate->Type,
                        candidate->Callee,
                        std::move(argument_values),
                        Value::CreateL(class_type, pointer, true));
                }
                else
                {
                    if (value)
                    {
                        value = builder.CreateCast(std::move(value), type);
                    }
                    else
                    {
                        const auto null = llvm::Constant::getNullValue(type->Gen(builder));
                        value = Value::CreateR(std::move(type), null);
                    }
                    builder.CreateStore(pointer, value);
                }
            }
            else
            {
                std::vector<Field> argument_fields;
                for (const auto &argument : arguments)
                    argument_fields.emplace_back(argument->AsField());

                const auto candidate = builder.FindFunction(
                    constructors,
                    argument_fields,
                    class_type,
                    self);
                Assert(candidate.has_value(), "no suitable candidate");

                builder.CreateCall(
                    candidate->Type,
                    candidate->Callee,
                    std::move(arguments),
                    Value::CreateL(class_type, pointer, true));
            }

            if (auto destructor = class_type->GetDestructor())
            {
                auto &reference = builder.GenFunction(
                    {
                        .Class = class_type,
                        .Mutable = destructor->Mutable,
                        .Expose = destructor->Expose,
                        .Name = destructor->Name,
                        .VarArg = destructor->VarArg,
                        .Result = destructor->Result,
                    });
                builder.PushDestructor(
                    pointer,
                    {
                        reference.Type->GenFunction(builder),
                        reference.Callee,
                    });
            }
        }
        else
        {
            if (!value)
            {
                Assert(arguments.empty(), "cannot construct non-class value");
                Assert(type != nullptr, "missing type");

                const auto null = llvm::Constant::getNullValue(type->Gen(builder));
                value = Value::CreateR(std::move(type), null);
            }
            else
            {
                value = builder.CreateCast(std::move(value), type);
            }

            builder.CreateStore(pointer, value);
        }

        storage = Value::CreateL(std::move(type), pointer, m_Info.Mutable);
    }

    builder.SetValue(m_Name, std::move(storage));
}

llove::StatementPtr llove::LetStatement::Reflect(Context &types) const
{
    Field info;
    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments(m_Arguments.size());

    m_Info.Reflect(types, info);

    if (m_Value)
        m_Value->Reflect(types, value);

    for (unsigned i = 0; i < m_Arguments.size(); ++i)
        m_Arguments.at(i)->Reflect(types, arguments.at(i));

    return std::make_unique<LetStatement>(std::move(info), m_Name, std::move(value), std::move(arguments));
}

std::ostream &llove::LetStatement::Print(std::ostream &stream) const
{
    m_Info.Print(stream << "let ", true, m_Name);
    if (m_Value)
    {
        stream << " = " << m_Value;
    }
    else if (!m_Arguments.empty())
    {
        stream << '(';
        for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
        {
            if (i != m_Arguments.begin())
                stream << ", ";
            stream << *i;
        }
        stream << ')';
    }
    return stream << ';';
}
