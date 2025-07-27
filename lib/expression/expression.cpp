#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::Expression::Expression(Location loc)
    : Statement(std::move(loc))
{
}

void llove::Expression::Gen(Builder &builder) const
{
    auto value = GenVal(builder, nullptr);
    const auto type = value->GetType();

    if (value->IsReferenceable() || !type->IsClass())
        return;

    const auto class_type = As<ClassType>(type);
    if (const auto destructor = class_type->GetDestructor())
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

        const auto pointer = builder.CreateAlloca(class_type);
        builder.CreateStore(pointer, value);
        value = Value::CreateL(class_type, pointer, true);

        builder.PushDestructor(
            value->GetPointer(),
            {
                reference.Type->GenFunction(builder),
                reference.Callee,
            });
    }
}

llove::CalleeInfo llove::Expression::GenCallee(Builder &builder) const
{
    const auto value = GenVal(builder, nullptr);

    const auto pointer_type = As<PointerType>(value->GetType());
    const auto is_function_pointer = pointer_type
                                     && !pointer_type->IsOpaque()
                                     && pointer_type->GetBase()->IsFunction();

    Assert(is_function_pointer, "not a function pointer");

    return {
        .Candidates = {
            FunctionReference
            {
                .Expose = false,
                .Name = {},
                .Type = As<FunctionType>(pointer_type->GetBase()),
                .Callee = value->Load(builder),
            }
        }
    };
}
