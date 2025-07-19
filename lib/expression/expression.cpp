#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

void llove::Expression::Gen(Builder &builder) const
{
    (void) GenVal(builder, nullptr);
}

llove::CalleeInfo llove::Expression::GenCallee(Builder &builder) const
{
    const auto value = GenVal(builder, nullptr);

    const auto pointer_type = As<PointerType>(value->GetType());
    const auto is_function_pointer = pointer_type
                                     && !pointer_type->IsOpaque()
                                     && pointer_type->GetBase()->GetId() == TypeId_Function;

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
