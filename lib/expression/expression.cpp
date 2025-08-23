#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::Expression::Expression(Location loc)
    : Statement(std::move(loc))
{
}

void llove::Expression::Gen(Builder &builder) const try
{
    const auto value = GenVal(builder, nullptr);
    const auto type = value->GetType();

    if (value->IsReference() || !type->IsClass())
        return;

    const auto class_type = As<ClassType>(type);
    if (const auto destructor = class_type->GetDestructor())
    {
        const auto pointer = builder.CreateAlloca(class_type->GenIR(builder));
        builder.CreateStore(value->Load(builder), pointer);

        const auto function = builder.GenFunction(
            {
                .Class = class_type,
                .Mutable = destructor->Mutable,
                .Expose = destructor->Expose,
                .Name = destructor->Name,
                .Variadic = { destructor->Variadic, {} },
                .Result = destructor->Result,
            });

        builder.PushDestructor(pointer, function);
    }
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::CalleeInfo llove::Expression::GenCallee(Builder &builder) const try
{
    const auto value = GenVal(builder, nullptr);
    auto type = As<FunctionType>(value->GetType());

    return {
        .Candidates = {
            FunctionReference
            {
                .Expose = false,
                .Name = {},
                .Type = std::move(type),
                .Callee = value->Load(builder),
            }
        }
    };
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}
