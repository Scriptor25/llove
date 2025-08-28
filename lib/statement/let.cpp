#include <llove/builder.hpp>
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
    Assert(m_Info.HasType() || m_Value != nullptr, "missing at least one of type or value");

    auto type = m_Info.HasType() ? m_Info.GetType() : nullptr;
    auto value = m_Value ? m_Value->GenVal(builder, type) : nullptr;

    if (!type)
        type = value->GetType();

    std::vector<ValuePtr> arguments;
    for (auto &argument : m_Arguments)
        arguments.emplace_back(argument->GenVal(builder, nullptr));

    builder.EmitLoc(m_Loc);

    llvm::Value *pointer;
    if (m_Info.IsReference())
    {
        Assert(arguments.empty(), "cannot construct reference");
        Assert(value != nullptr, "missing initializer value");
        Assert(value->IsReference(), "reference from rvalue");
        Assert(type == value->GetType(), "reference type mismatch");
        Assert(!m_Info.IsMutable() || value->IsMutable(), "reference mutability violation");

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
                const auto candidate = builder.FindFunction(
                    constructors,
                    { value->AsField() },
                    self->AsField(),
                    true);
                if (candidate.has_value())
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
                Assert(arguments.empty(), "illegal arguments for implicit default constructor");

                builder.CreateStore(llvm::Constant::getNullValue(type->GenIR(builder)), pointer);
            }
            else
            {
                std::vector<Field> argument_fields;
                for (const auto &argument : arguments)
                    argument_fields.emplace_back(argument->AsField());

                const auto candidate = builder.FindFunction(
                    constructors,
                    argument_fields,
                    self->AsField(),
                    false);
                Assert(candidate.has_value(), "no suitable candidate");

                builder.CreateCall(*candidate, std::move(arguments), self);
            }

            builder.PushDestructor(pointer, class_type);
        }
        else
        {
            if (!value)
            {
                Assert(arguments.empty(), "cannot construct non-class value");

                value = Value::CreateR(type, llvm::Constant::getNullValue(type->GenIR(builder)));
            }
            else if (type)
            {
                value = builder.CreateCast(std::move(value), type, true);
            }

            builder.CreateStore(value->Load(builder), pointer);
        }
    }

    auto storage = Value::CreateL(std::move(type), pointer, m_Info.IsMutable());
    builder.GetDebug().CreateVariable(builder, m_Name, storage);
    builder.SetValue(m_Name, std::move(storage));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::StatementPtr llove::LetStatement::Reflect(Context &context) const try
{
    Field info;
    ExpressionPtr value;
    std::vector<ExpressionPtr> arguments;

    m_Info.Reflect(context, info);

    if (m_Value)
        m_Value->Reflect(context, value);

    for (auto &argument : m_Arguments)
        argument->Reflect(context, arguments.emplace_back());

    return std::make_unique<LetStatement>(m_Loc, std::move(info), m_Name, std::move(value), std::move(arguments));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::LetStatement::Print(std::ostream &stream) const
{
    m_Info.Print(stream << "let ", true, m_Name);

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
