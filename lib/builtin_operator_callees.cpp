#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/operator.hpp>
#include <llove/value.hpp>

static llove::ValuePtr operator_neg(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    const auto type = operand->GetType();

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateNeg(operand->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFNeg(operand->Load(builder));
        break;
    default:
        llove::Error("operator '-{}' not implemented", operand->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_not(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    const auto type = operand->GetType();

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
    case llove::TypeId_Float:
    case llove::TypeId_Pointer:
        result = builder.CreateIsNull(operand->Load(builder));
        break;
    default:
        llove::Error("operator '!{}' not implemented", operand->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

static llove::ValuePtr operator_inv(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    const auto type = operand->GetType();

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateNot(operand->Load(builder));
        break;
    default:
        llove::Error("operator '~{}' not implemented", operand->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_inc(llove::Builder &builder, llove::ValuePtr operand, const bool suffix)
{
    const auto pre = suffix ? operand->Load(builder) : nullptr;

    auto type = operand->GetType();

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
    {
        const auto offset = llvm::ConstantInt::get(type->GenIR(builder), 1u, false);
        result = builder.CreateAdd(operand->Load(builder), offset);
        break;
    }
    case llove::TypeId_Float:
    {
        const auto offset = llvm::ConstantFP::get(type->GenIR(builder), 1.0);
        result = builder.CreateFAdd(operand->Load(builder), offset);
        break;
    }
    case llove::TypeId_Pointer:
    {
        const auto base_type = llove::As<llove::PointerType>(type)->GetBase();
        const auto offset = llvm::ConstantInt::get(builder.GetPointerSizeType(), 1u, false);
        result = builder.CreateGEP(base_type->GenIR(builder), operand->Load(builder), offset);
        break;
    }
    default:
        llove::Error("operator '{}{}{}' not implemented", suffix ? "" : "++", operand->AsField(), suffix ? "++" : "");
    }

    operand->Store(builder, result);

    if (suffix)
        return llove::Value::CreateR(std::move(type), pre);

    return operand;
}

static llove::ValuePtr operator_dec(llove::Builder &builder, llove::ValuePtr operand, const bool suffix)
{
    const auto pre = suffix ? operand->Load(builder) : nullptr;

    auto type = operand->GetType();

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
    {
        const auto offset = llvm::ConstantInt::get(type->GenIR(builder), 1, false);
        result = builder.CreateSub(operand->Load(builder), offset);
        break;
    }
    case llove::TypeId_Float:
    {
        const auto offset = llvm::ConstantFP::get(type->GenIR(builder), 1.0);
        result = builder.CreateFSub(operand->Load(builder), offset);
        break;
    }
    case llove::TypeId_Pointer:
    {
        const auto base_type = llove::As<llove::PointerType>(type)->GetBase();
        const auto offset = llvm::ConstantInt::get(builder.GetPointerSizeType(), -1, true);
        result = builder.CreateGEP(base_type->GenIR(builder), operand->Load(builder), offset);
        break;
    }
    default:
        llove::Error("operator '{}{}{}' not implemented", suffix ? "" : "--", operand->AsField(), suffix ? "--" : "");
    }

    operand->Store(builder, result);

    if (suffix)
        return llove::Value::CreateR(std::move(type), pre);

    return operand;
}

static llove::ValuePtr operator_deref(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    llove::Assert(operand->GetType()->IsPointer(), "illegal dereference of non-pointer value");
    const auto type = llove::As<llove::PointerType>(operand->GetType());
    return llove::Value::CreateL(type->GetBase(), operand->Load(builder), type->IsMutable());
}

static llove::ValuePtr operator_ref(llove::Builder &builder, const llove::ValuePtr &operand, bool /*suffix*/)
{
    return operand->Reference(builder);
}

static llove::ValuePtr operator_copy(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    right = builder.CreateCast(std::move(right), left->GetType(), true);
    left->Store(builder, right);
    return left;
}

static llove::ValuePtr operator_add(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    if ((left_type->IsPointer() && right_type->IsInteger()) || (left_type->IsInteger() && right_type->IsPointer()))
    {
        llove::PointerType::Ptr type;
        llove::ValuePtr base, offset;

        if (left_type->IsPointer())
        {
            type = llove::As<llove::PointerType>(left_type);
            base = std::move(left);
            offset = std::move(right);
        }
        else
        {
            type = llove::As<llove::PointerType>(right_type);
            base = std::move(right);
            offset = std::move(left);
        }

        const auto base_type = type->GetBase();
        const auto pointer = builder.CreateGEP(
            base_type->GenIR(builder),
            base->Load(builder),
            offset->Load(builder));

        return llove::Value::CreateR(std::move(type), pointer);
    }

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateAdd(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFAdd(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} + {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_sub(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    if (left_type->IsPointer() && right_type->IsInteger())
    {
        const auto offset = builder.CreateNeg(right->Load(builder));
        const auto base_type = llove::As<llove::PointerType>(left_type)->GetBase();

        const auto pointer = builder.CreateGEP(
            base_type->GenIR(builder),
            left->Load(builder),
            offset);

        return llove::Value::CreateR(left_type, pointer);
    }
    if (left_type->IsPointer() && right_type->IsPointer())
    {
        const auto base_type = llove::As<llove::PointerType>(left_type)->GetBase();
        const auto value = builder.CreatePtrDiff(base_type->GenIR(builder), left->Load(builder), right->Load(builder));
        auto int_type = builder.GetContext().GetInteger(true, builder.GetDataLayout().getPointerSizeInBits());
        return llove::Value::CreateR(std::move(int_type), value);
    }

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateSub(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFSub(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} - {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_mul(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateMul(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFMul(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} * {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_div(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *value;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        value = builder.CreateDiv(
            llove::As<llove::IntegerType>(type)->IsSigned(),
            left->Load(builder),
            right->Load(builder));
        break;
    case llove::TypeId_Float:
        value = builder.CreateFDiv(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} / {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, value);
}

static llove::ValuePtr operator_rem(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateRem(
            llove::As<llove::IntegerType>(type)->IsSigned(),
            left->Load(builder),
            right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFRem(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} % {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_and(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateAnd(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} & {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_or(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateOr(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} | {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_xor(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateXor(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} ^ {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(type, result);
}

static llove::ValuePtr operator_eq(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateCmpEQ(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFCmpEQ(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Pointer:
        result = builder.CreatePCmpEQ(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} == {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

static llove::ValuePtr operator_ne(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateCmpNE(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFCmpNE(left->Load(builder), right->Load(builder));
        break;
    case llove::TypeId_Pointer:
        result = builder.CreatePCmpNE(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} != {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

static llove::ValuePtr operator_lt(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateCmpLT(
            llove::As<llove::IntegerType>(type)->IsSigned(),
            left->Load(builder),
            right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFCmpLT(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} < {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

static llove::ValuePtr operator_le(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateCmpLE(
            llove::As<llove::IntegerType>(type)->IsSigned(),
            left->Load(builder),
            right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFCmpLE(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} <= {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

static llove::ValuePtr operator_gt(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateCmpGT(
            llove::As<llove::IntegerType>(type)->IsSigned(),
            left->Load(builder),
            right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFCmpGT(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} > {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

static llove::ValuePtr operator_ge(llove::Builder &builder, llove::ValuePtr left, llove::ValuePtr right)
{
    const auto left_type = left->GetType();
    const auto right_type = right->GetType();

    const auto type = builder.GetContext().TypeUnion(left_type, right_type);
    left = builder.CreateCast(std::move(left), type, true);
    right = builder.CreateCast(std::move(right), type, true);

    llvm::Value *result;
    switch (type->GetId())
    {
    case llove::TypeId_Integer:
        result = builder.CreateCmpGE(
            llove::As<llove::IntegerType>(type)->IsSigned(),
            left->Load(builder),
            right->Load(builder));
        break;
    case llove::TypeId_Float:
        result = builder.CreateFCmpGE(left->Load(builder), right->Load(builder));
        break;
    default:
        llove::Error("operator '{} >= {}' not implemented", left->AsField(), right->AsField());
    }

    return llove::Value::CreateR(builder.GetContext().GetBoolean(), result);
}

const std::map<std::string_view, llove::BIOperator<1>::CalleeType> llove::BIUnOperatorCallees
{
    { "-", operator_neg },
    { "!", operator_not },
    { "~", operator_inv },

    { "++", operator_inc },
    { "--", operator_dec },

    { "*", operator_deref },
    { "&", operator_ref },
};

const std::map<std::string_view, llove::BIOperator<2>::CalleeType> llove::BIBiOperatorCallees
{
    { "=", operator_copy },

    { "+", operator_add },
    { "-", operator_sub },
    { "*", operator_mul },
    { "/", operator_div },
    { "%", operator_rem },

    { "&", operator_and },
    { "|", operator_or },
    { "^", operator_xor },

    { "==", operator_eq },
    { "!=", operator_ne },
    { "<", operator_lt },
    { "<=", operator_le },
    { ">", operator_gt },
    { ">=", operator_ge },
};
