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

    const auto pointer = builder.CreateAlloca(type->GenIR(builder));
    builder.CreateStore(value->Load(builder), pointer);
    builder.PushDestructor(pointer, As<ClassType>(type));
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
        {
            {
                .Name = {},
                .Type = std::move(type),
                .Callee = value->Load(builder),
            }
        },
        {},
    };
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}
