#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/value.hpp>

llove::ValuePtr llove::Builder::CreateCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateICmpEQ(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateICmpNE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpLT(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_LLVMBuilder.CreateICmpSLT(left->Load(*this), right->Load(*this));
    else
        value = m_LLVMBuilder.CreateICmpULT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpGT(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_LLVMBuilder.CreateICmpSGT(left->Load(*this), right->Load(*this));
    else
        value = m_LLVMBuilder.CreateICmpUGT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpLE(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_LLVMBuilder.CreateICmpSLE(left->Load(*this), right->Load(*this));
    else
        value = m_LLVMBuilder.CreateICmpULE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateCmpGE(const ValuePtr &left, const ValuePtr &right)
{
    llvm::Value *value;
    if (As<IntegerType>(left->GetType())->IsSigned())
        value = m_LLVMBuilder.CreateICmpSGE(left->Load(*this), right->Load(*this));
    else
        value = m_LLVMBuilder.CreateICmpUGE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateFCmpOEQ(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateFCmpONE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpLT(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateFCmpOLT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpGT(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateFCmpOGT(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpLE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateFCmpOLE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateFCmpGE(const ValuePtr &left, const ValuePtr &right)
{
    const auto value = m_LLVMBuilder.CreateFCmpOGE(left->Load(*this), right->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreatePCmpEQ(const ValuePtr &left, const ValuePtr &right)
{
    const auto type = GetPointerSizeType();
    const auto left_int = m_LLVMBuilder.CreatePtrToInt(left->Load(*this), type);
    const auto right_int = m_LLVMBuilder.CreatePtrToInt(right->Load(*this), type);
    const auto value = m_LLVMBuilder.CreateICmpEQ(left_int, right_int);
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreatePCmpNE(const ValuePtr &left, const ValuePtr &right)
{
    const auto type = GetPointerSizeType();
    const auto left_int = m_LLVMBuilder.CreatePtrToInt(left->Load(*this), type);
    const auto right_int = m_LLVMBuilder.CreatePtrToInt(right->Load(*this), type);
    const auto value = m_LLVMBuilder.CreateICmpNE(left_int, right_int);
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateNeg(const ValuePtr &operand)
{
    const auto value = m_LLVMBuilder.CreateNeg(operand->Load(*this));
    return Value::CreateR(operand->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateFNeg(const ValuePtr &operand)
{
    const auto value = m_LLVMBuilder.CreateFNeg(operand->Load(*this));
    return Value::CreateR(operand->GetType(), value);
}

llove::ValuePtr llove::Builder::CreateNot(const ValuePtr &operand)
{
    const auto value = m_LLVMBuilder.CreateIsNull(operand->Load(*this));
    return Value::CreateR(m_Context.GetInteger(false, 1), value);
}

llove::ValuePtr llove::Builder::CreateInv(const ValuePtr &operand)
{
    const auto value = m_LLVMBuilder.CreateNot(operand->Load(*this));
    return Value::CreateR(operand->GetType(), value);
}
