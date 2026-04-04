#include <ranges>

#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::LambdaExpression::LambdaExpression(
    Location loc,
    std::optional<Capture> default_capture,
    std::map<std::string, Capture> captures,
    std::vector<Parameter> parameters,
    Variadic variadic,
    Field result,
    StatementPtr content)
    : Expression(std::move(loc)),
      m_DefaultCapture(std::move(default_capture)),
      m_Captures(std::move(captures)),
      m_Parameters(std::move(parameters)),
      m_Variadic(std::move(variadic)),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

llove::ValuePtr llove::LambdaExpression::GenVal(Builder &builder, TypePtr /* expect */) const
{
    auto &context = builder.GetContext();

    std::map<std::string, std::pair<llvm::Value *, ClassMemberReference>> capture_map;
    auto require_mutable = false;

    for (const auto &[name, capture] : m_Captures)
    {
        auto &[loc, is_mutable, is_reference] = capture;

        Assert(!capture_map.contains(name), loc, "illegal re-capture of already captured value '{}'", name);
        Assert(builder.HasValue(name), loc, "illegal capture of undefined value '{}'", name);

        const auto value = builder.GetValue(name);
        auto &[cap_value, cap_reference] = capture_map[name];

        require_mutable |= !is_reference && is_mutable;

        if (is_reference)
        {
            Assert(
                !is_mutable || value->IsMutable(),
                loc,
                "illegal mutable capture of immutable reference to value '{}'",
                name);

            cap_value = value->GetPointer();
        }
        else
        {
            cap_value = value->Load(builder);
        }

        cap_reference = {
            .Info = Field
            {
                is_mutable,
                is_reference,
                value->GetType(),
            },
            .Name = name,
        };
    }

    if (m_DefaultCapture)
    {
        auto &[loc, is_mutable, is_reference] = *m_DefaultCapture;

        require_mutable |= !is_reference && is_mutable;

        for (auto li = builder.GetStackTop(); li != builder.GetStackBottom(); ++li)
            for (auto &[name, value] : li->Values)
            {
                if (capture_map.contains(name))
                    continue;

                auto &[cap_value, cap_reference] = capture_map[name];

                auto as_mutable = is_mutable;
                if (is_reference)
                {
                    if (is_mutable && !value->IsMutable())
                        as_mutable = false;

                    cap_value = value->GetPointer();
                }
                else
                {
                    cap_value = value->Load(builder);
                }

                cap_reference = {
                    .Info = Field
                    {
                        as_mutable,
                        is_reference,
                        value->GetType(),
                    },
                    .Name = name,
                };
            }
    }

    const auto class_name = "lambda" + std::to_string(std::hash<Location>()(m_Loc));
    const auto class_type = context.GetClass(class_name);

    std::vector<ClassMemberReference> members;
    for (auto &[value, reference] : capture_map | std::views::values)
        members.push_back(std::move(reference));
    class_type->SetMembers(std::move(members));

    std::vector<Field> parameters(m_Parameters.size());
    for (size_t i = 0; i < m_Parameters.size(); ++i)
        parameters[i] = m_Parameters[i].Info;

    class_type->SetFunctions(
        {
            ClassFunctionReference
            {
                .IsPublic = true,
                .IsMutable = require_mutable,
                .Name = "()",
                .Parameters = parameters,
                .IsVariadic = m_Variadic.Is,
                .Result = m_Result,
            },
        });

    StatementPtr content;
    m_Content->Reflect(context, content);

    const Function agg
    {
        .Loc = m_Loc,
        .IsPublic = true,
        .IsMutable = require_mutable,
        .Class = class_type,
        .Name = "()",
        .Parameters = m_Parameters,
        .Variadic = m_Variadic,
        .Result = m_Result,
        .Content = std::move(content),
    };
    builder.GenFunction(agg, false);

    const auto gen_type = class_type->GenIR(builder);
    llvm::Value *aggregate = llvm::Constant::getNullValue(gen_type);

    for (auto &[name, capture] : capture_map)
    {
        const auto index = class_type->GetMemberIndex(name);
        aggregate = builder.CreateInsertValue(aggregate, capture.first, index);
    }

    return Value::CreateR(class_type, aggregate);
}

llove::CalleeInfo llove::LambdaExpression::GenCallee(Builder &builder) const
{
    auto value = GenVal(builder, nullptr);

    auto candidates = builder.GetFunctions("()", value->AsField());

    return { std::move(candidates), std::move(value) };
}

llove::StatementPtr llove::LambdaExpression::Reflect(Context &context) const
{
    std::vector<Parameter> parameters(m_Parameters.size());
    for (size_t i = 0; i < m_Parameters.size(); ++i)
        m_Parameters[i].Reflect(context, parameters[i]);

    Field result;
    m_Result.Reflect(context, result);

    StatementPtr content;
    m_Content->Reflect(context, content);

    return std::make_unique<LambdaExpression>(
        m_Loc,
        m_DefaultCapture,
        m_Captures,
        std::move(parameters),
        m_Variadic,
        std::move(result),
        std::move(content));
}

std::ostream &llove::LambdaExpression::Print(std::ostream &stream) const
{
    stream << '[';
    if (m_DefaultCapture)
    {
        stream
                << (m_DefaultCapture->IsMutable ? "mut " : "")
                << (m_DefaultCapture->IsReference ? "&" : "=");
    }
    for (auto i = m_Captures.begin(); i != m_Captures.end(); ++i)
    {
        if (m_DefaultCapture || i != m_Captures.begin())
            stream << ", ";

        stream
                << (i->second.IsMutable ? "mut " : "")
                << (i->second.IsReference ? "&" : "")
                << i->first;
    }
    stream << "](";
    for (auto i = m_Parameters.begin(); i != m_Parameters.end(); ++i)
    {
        if (i != m_Parameters.begin())
            stream << ", ";

        i->Print(stream);
    }
    if (m_Variadic.Is)
    {
        if (!m_Parameters.empty())
            stream << ", ";
        stream << "..." << m_Variadic.Name;
    }
    stream << ')';

    if (!m_Result.GetType()->IsVoid())
        m_Result.Print(stream << ": ");

    return stream << ' ' << m_Content;
}
