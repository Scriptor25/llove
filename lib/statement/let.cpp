#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::LetStatement::LetStatement(
    Location loc,
    Field info,
    std::string name,
    ExpressionPtr value,
    std::vector<ExpressionPtr> arguments)
    : Statement(std::move(loc)),
      m_Info(std::move(info)),
      m_Name(std::move(name)),
      m_Value(std::move(value)),
      m_Arguments(std::move(arguments))
{
}

void llove::LetStatement::Gen(Builder &builder) const try
{
    Assert(m_Info.Type != nullptr || m_Value != nullptr, "missing type or value");

    auto value = m_Value ? m_Value->GenVal(builder, m_Info.Type) : nullptr;
    auto type = m_Info.Type ? m_Info.Type : value->GetType();

    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
        arguments.emplace_back(argument->GenVal(builder, nullptr));

    builder.EmitLoc(m_Loc);

    llvm::Value *pointer;
    if (m_Info.Reference)
    {
        Assert(arguments.empty(), "cannot construct reference");
        Assert(value != nullptr, "missing initializer value");
        Assert(value->IsReferenceable(), "reference from rvalue");
        Assert(type == value->GetType(), "reference type mismatch");
        Assert(!m_Info.Mutable || value->IsMutable(), "reference mutability violation");

        pointer = value->GetPointer();
    }
    else
    {
        pointer = builder.CreateAlloca(type);

        if (type->IsClass())
        {
            const auto self = Value::CreateL(type, pointer, true);

            const auto class_type = As<ClassType>(type);
            const auto constructors = class_type->GetConstructors();

            if (value)
            {
                const auto candidate = builder.FindFunction(
                    constructors,
                    { value->AsField() },
                    class_type,
                    self->AsField(),
                    true);
                if (candidate.has_value())
                {
                    builder.CreateCall(*candidate, { std::move(value) }, self);
                }
                else
                {
                    Assert(!value->IsReferenceable(), "illegal implicit copy");
                    value = builder.CreateCast(std::move(value), type, true);
                    builder.CreateStore(pointer, value);
                }
            }
            else if (constructors.empty())
            {
                Assert(arguments.empty(), "invalid arguments for implicit default constructor");

                const auto null = llvm::Constant::getNullValue(type->Gen(builder));
                builder.CreateStore(pointer, null);
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
                    self->AsField(),
                    false);
                Assert(candidate.has_value(), "no suitable candidate");

                builder.CreateCall(*candidate, std::move(arguments), self);
            }

            if (auto destructor = class_type->GetDestructor())
            {
                const auto &reference = builder.GenFunction(
                    {
                        .Class = class_type,
                        .Mutable = destructor->Mutable,
                        .Expose = destructor->Expose,
                        .Name = destructor->Name,
                        .VarArg = destructor->VarArg,
                        .Result = destructor->Result,
                    });

                builder.PushDestructor(pointer, reference);
            }
        }
        else
        {
            if (!value)
            {
                Assert(arguments.empty(), "cannot construct non-class value");
                Assert(type != nullptr, "missing type");

                const auto null = llvm::Constant::getNullValue(type->Gen(builder));
                value = Value::CreateR(type, null);
            }
            else
            {
                value = builder.CreateCast(std::move(value), type, true);
            }

            builder.CreateStore(pointer, value);
        }
    }

    auto storage = Value::CreateL(std::move(type), pointer, m_Info.Mutable);
    builder.CreateDbgVariable(m_Name, storage);
    builder.SetValue(m_Name, std::move(storage));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
}

llove::StatementPtr llove::LetStatement::Reflect(Builder &builder) const try
{
    Field info;
    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments(m_Arguments.size());

    m_Info.Reflect(builder, info);

    if (m_Value)
        m_Value->Reflect(builder, value);

    for (unsigned i = 0; i < m_Arguments.size(); ++i)
        m_Arguments.at(i)->Reflect(builder, arguments.at(i));

    return std::make_unique<LetStatement>(m_Loc, std::move(info), m_Name, std::move(value), std::move(arguments));
}
catch (const std::shared_ptr<ErrorStack> &cause)
{
    throw std::make_shared<ErrorStack>(cause, m_Loc, std::nullopt);
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
