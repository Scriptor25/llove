#include <llove/builder.hpp>

llvm::ConstantInt *llove::Builder::GetI1(const bool value)
{
    return m_LLVMBuilder.getInt1(value);
}

llvm::ConstantInt *llove::Builder::GetI8(const uint8_t value)
{
    return m_LLVMBuilder.getInt8(value);
}

llvm::ConstantInt *llove::Builder::GetI16(const uint16_t value)
{
    return m_LLVMBuilder.getInt16(value);
}

llvm::ConstantInt *llove::Builder::GetI32(const uint32_t value)
{
    return m_LLVMBuilder.getInt32(value);
}

llvm::ConstantInt *llove::Builder::GetI64(const uint64_t value)
{
    return m_LLVMBuilder.getInt64(value);
}

llvm::Constant *llove::Builder::GetStr(const std::string &value, const std::string &name)
{
    if (m_Strings.contains(value))
        return m_Strings.at(value);
    return m_Strings[value] = m_LLVMBuilder.CreateGlobalString(value, name, 0, &m_LLVMModule);
}

void llove::Builder::SetCurrentDebugLocation(llvm::DebugLoc loc)
{
    m_LLVMBuilder.SetCurrentDebugLocation(std::move(loc));
}

void llove::Builder::SetInsertPoint(llvm::BasicBlock *block)
{
    Assert(block != nullptr, "block must not be null");

    m_LLVMBuilder.SetInsertPoint(block);
}

void llove::Builder::SetInsertPoint(llvm::Instruction *instruction)
{
    Assert(instruction != nullptr, "instruction must not be null");

    m_LLVMBuilder.SetInsertPoint(instruction);
}

void llove::Builder::ClearInsertionPoint()
{
    m_LLVMBuilder.ClearInsertionPoint();
}

llvm::BasicBlock *llove::Builder::GetInsertBlock() const
{
    return m_LLVMBuilder.GetInsertBlock();
}

llvm::AllocaInst *llove::Builder::CreateAlloca(
    llvm::Type *type,
    llvm::Function *parent,
    llvm::Value *array_size,
    const std::string &name)
{
    Assert(type != nullptr, "type must not be null");

    const auto block = m_LLVMBuilder.GetInsertBlock();
    Assert(parent != nullptr || block != nullptr, "parent must not be null");
    m_LLVMBuilder.SetInsertPointPastAllocas(parent ? parent : block->getParent());
    const auto pointer = m_LLVMBuilder.CreateAlloca(type, array_size, name);
    m_LLVMBuilder.SetInsertPoint(block);
    return pointer;
}

llvm::LoadInst *llove::Builder::CreateLoad(
    llvm::Type *type,
    llvm::Value *pointer,
    const bool is_volatile,
    const std::string &name)
{
    Assert(type != nullptr, "type must not be null");
    Assert(pointer != nullptr, "pointer must not be null");

    return m_LLVMBuilder.CreateLoad(type, pointer, is_volatile, name);
}

llvm::StoreInst *llove::Builder::CreateStore(llvm::Value *value, llvm::Value *pointer, const bool is_volatile)
{
    Assert(value != nullptr, "value must not be null");
    Assert(pointer != nullptr, "pointer must not be null");

    return m_LLVMBuilder.CreateStore(value, pointer, is_volatile);
}

llvm::Value *llove::Builder::CreateArrayGEP(
    llvm::Type *type,
    llvm::Value *pointer,
    const unsigned index,
    const std::string &name)
{
    Assert(type != nullptr, "type must not be null");
    Assert(pointer != nullptr, "pointer must not be null");

    return m_LLVMBuilder.CreateConstInBoundsGEP2_64(type, pointer, 0, index, name);
}

llvm::Value *llove::Builder::CreateStructGEP(
    llvm::Type *type,
    llvm::Value *pointer,
    const unsigned index,
    const std::string &name)
{
    Assert(type != nullptr, "type must not be null");
    Assert(pointer != nullptr, "pointer must not be null");

    return m_LLVMBuilder.CreateStructGEP(type, pointer, index, name);
}

llvm::Value *llove::Builder::CreateGEP(
    llvm::Type *type,
    llvm::Value *pointer,
    const unsigned index,
    const std::string &name)
{
    Assert(type != nullptr, "type must not be null");
    Assert(pointer != nullptr, "pointer must not be null");

    return m_LLVMBuilder.CreateConstGEP1_64(type, pointer, index, name);
}

llvm::Value *llove::Builder::CreateGEP(
    llvm::Type *type,
    llvm::Value *pointer,
    llvm::Value *index,
    const std::string &name,
    const bool is_in_bounds)
{
    Assert(type != nullptr, "type must not be null");
    Assert(pointer != nullptr, "pointer must not be null");
    Assert(index != nullptr, "index must not be null");

    return m_LLVMBuilder.CreateGEP(type, pointer, index, name, is_in_bounds);
}

llvm::Value *llove::Builder::CreateExtractValue(llvm::Value *aggregate, const unsigned index, const std::string &name)
{
    Assert(aggregate != nullptr, "aggregate must not be null");

    return m_LLVMBuilder.CreateExtractValue(aggregate, index, name);
}

llvm::Value *llove::Builder::CreateInsertValue(
    llvm::Value *aggregate,
    llvm::Value *value,
    const unsigned index,
    const std::string &name)
{
    Assert(aggregate != nullptr, "aggregate must not be null");
    Assert(value != nullptr, "value must not be null");

    return m_LLVMBuilder.CreateInsertValue(aggregate, value, index, name);
}

llvm::Value *llove::Builder::CreateAdd(
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateAdd(left, right, name);
}

llvm::Value *llove::Builder::CreateSub(
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateSub(left, right, name);
}

llvm::Value *llove::Builder::CreateMul(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateMul(left, right, name);
}

llvm::Value *llove::Builder::CreateDiv(
    const bool is_signed,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name,
    const bool is_exact)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    if (is_signed)
        return m_LLVMBuilder.CreateSDiv(left, right, name, is_exact);
    return m_LLVMBuilder.CreateUDiv(left, right, name, is_exact);
}

llvm::Value *llove::Builder::CreateRem(
    const bool is_signed,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    if (is_signed)
        return m_LLVMBuilder.CreateSRem(left, right, name);
    return m_LLVMBuilder.CreateURem(left, right, name);
}

llvm::Value *llove::Builder::CreateAnd(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateAnd(left, right, name);
}

llvm::Value *llove::Builder::CreateOr(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateOr(left, right, name);
}

llvm::Value *llove::Builder::CreateXor(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateXor(left, right, name);
}

llvm::Value *llove::Builder::CreateLogicalAnd(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateLogicalAnd(left, right, name);
}

llvm::Value *llove::Builder::CreateLogicalOr(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateLogicalOr(left, right, name);
}

llvm::Value *llove::Builder::CreateFAdd(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFAdd(left, right, name);
}

llvm::Value *llove::Builder::CreateFSub(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFSub(left, right, name);
}

llvm::Value *llove::Builder::CreateFMul(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFMul(left, right, name);
}

llvm::Value *llove::Builder::CreateFDiv(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFDiv(left, right, name);
}

llvm::Value *llove::Builder::CreateFRem(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFRem(left, right, name);
}

llvm::Value *llove::Builder::CreateNeg(llvm::Value *value, const std::string &name)
{
    Assert(value != nullptr, "operand must not be null");

    return m_LLVMBuilder.CreateNeg(value, name);
}

llvm::Value *llove::Builder::CreateFNeg(llvm::Value *value, const std::string &name)
{
    Assert(value != nullptr, "operand must not be null");

    return m_LLVMBuilder.CreateFNeg(value, name);
}

llvm::Value *llove::Builder::CreateNot(llvm::Value *value, const std::string &name)
{
    Assert(value != nullptr, "operand must not be null");

    return m_LLVMBuilder.CreateNot(value, name);
}

llvm::Value *llove::Builder::CreateCmpEQ(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateICmpEQ(left, right, name);
}

llvm::Value *llove::Builder::CreateCmpNE(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateICmpNE(left, right, name);
}

llvm::Value *llove::Builder::CreateCmpLT(
    const bool is_signed,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    if (is_signed)
        return m_LLVMBuilder.CreateICmpSLT(left, right, name);
    return m_LLVMBuilder.CreateICmpULT(left, right, name);
}

llvm::Value *llove::Builder::CreateCmpLE(
    const bool is_signed,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    if (is_signed)
        return m_LLVMBuilder.CreateICmpSLE(left, right, name);
    return m_LLVMBuilder.CreateICmpULE(left, right, name);
}

llvm::Value *llove::Builder::CreateCmpGT(
    const bool is_signed,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    if (is_signed)
        return m_LLVMBuilder.CreateICmpSGT(left, right, name);
    return m_LLVMBuilder.CreateICmpUGT(left, right, name);
}

llvm::Value *llove::Builder::CreateCmpGE(
    const bool is_signed,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    if (is_signed)
        return m_LLVMBuilder.CreateICmpSGE(left, right, name);
    return m_LLVMBuilder.CreateICmpUGE(left, right, name);
}

llvm::Value *llove::Builder::CreateFCmpEQ(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFCmpOEQ(left, right, name);
}

llvm::Value *llove::Builder::CreateFCmpNE(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFCmpONE(left, right, name);
}

llvm::Value *llove::Builder::CreateFCmpLT(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFCmpOLT(left, right, name);
}

llvm::Value *llove::Builder::CreateFCmpLE(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFCmpOLE(left, right, name);
}

llvm::Value *llove::Builder::CreateFCmpGT(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFCmpOGT(left, right, name);
}

llvm::Value *llove::Builder::CreateFCmpGE(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreateFCmpOGE(left, right, name);
}

llvm::Value *llove::Builder::CreatePCmpEQ(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    const auto type = GetPointerSizeType();
    const auto left_int = m_LLVMBuilder.CreatePtrToInt(left, type);
    const auto right_int = m_LLVMBuilder.CreatePtrToInt(right, type);
    return m_LLVMBuilder.CreateICmpEQ(left_int, right_int, name);
}

llvm::Value *llove::Builder::CreatePCmpNE(llvm::Value *left, llvm::Value *right, const std::string &name)
{
    const auto type = GetPointerSizeType();
    const auto left_int = m_LLVMBuilder.CreatePtrToInt(left, type);
    const auto right_int = m_LLVMBuilder.CreatePtrToInt(right, type);
    return m_LLVMBuilder.CreateICmpNE(left_int, right_int, name);
}

llvm::Value *llove::Builder::CreatePtrDiff(
    llvm::Type *element_type,
    llvm::Value *left,
    llvm::Value *right,
    const std::string &name)
{
    Assert(element_type != nullptr, "element type must not be null");
    Assert(left != nullptr, "left must not be null");
    Assert(right != nullptr, "right must not be null");

    return m_LLVMBuilder.CreatePtrDiff(element_type, left, right, name);
}

llvm::Value *llove::Builder::CreateIsNotNull(llvm::Value *value, const std::string &name)
{
    Assert(value != nullptr, "value must not be null");

    return m_LLVMBuilder.CreateIsNotNull(value, name);
}

llvm::Value *llove::Builder::CreateIsNull(llvm::Value *value, const std::string &name)
{
    Assert(value != nullptr, "value must not be null");

    return m_LLVMBuilder.CreateIsNull(value, name);
}

llvm::BranchInst *llove::Builder::CreateBranch(llvm::BasicBlock *block)
{
    Assert(block != nullptr, "block must not be null");

    return m_LLVMBuilder.CreateBr(block);
}

llvm::BranchInst *llove::Builder::CreateBranch(
    llvm::Value *condition,
    llvm::BasicBlock *true_block,
    llvm::BasicBlock *false_block)
{
    Assert(condition != nullptr, "condition must not be null");
    Assert(true_block != nullptr, "true block must not be null");
    Assert(false_block != nullptr, "false block must not be null");

    return m_LLVMBuilder.CreateCondBr(condition, true_block, false_block);
}

llvm::SwitchInst *llove::Builder::CreateSwitch(
    llvm::Value *condition,
    llvm::BasicBlock *default_block,
    const std::map<llvm::ConstantInt *, llvm::BasicBlock *> &cases)
{
    Assert(condition != nullptr, "condition must not be null");
    Assert(default_block != nullptr, "default block must not be null");

    const auto instruction = m_LLVMBuilder.CreateSwitch(condition, default_block, cases.size());
    for (auto &[key, block] : cases)
        instruction->addCase(key, block);
    return instruction;
}

llvm::PHINode *llove::Builder::CreatePHI(llvm::Type *type, const std::map<llvm::BasicBlock *, llvm::Value *> &operands)
{
    Assert(type != nullptr, "type must not be null");

    const auto instruction = m_LLVMBuilder.CreatePHI(type, operands.size());
    for (auto &[block, value] : operands)
        instruction->addIncoming(value, block);
    return instruction;
}

llvm::CallInst *llove::Builder::CreateCall(
    llvm::FunctionType *type,
    llvm::Value *callee,
    const std::vector<llvm::Value *> &arguments,
    const std::string &name)
{
    Assert(type != nullptr, "type must not be null");
    Assert(callee != nullptr, "callee must not be null");

    return m_LLVMBuilder.CreateCall(type, callee, arguments, name);
}

llvm::CallInst *llove::Builder::CreateMemcpy(llvm::Value *dst, llvm::Value *src, llvm::Value *count)
{
    Assert(dst != nullptr, "dst must not be null");
    Assert(src != nullptr, "src must not be null");
    Assert(count != nullptr, "count must not be null");

    return m_LLVMBuilder.CreateIntrinsic(
        llvm::Intrinsic::memcpy,
        {
            dst->getType(),
            src->getType(),
            count->getType(),
        },
        {
            dst,
            src,
            count,
        });
}

llvm::ReturnInst *llove::Builder::CreateRetVoid()
{
    return m_LLVMBuilder.CreateRetVoid();
}

llvm::ReturnInst *llove::Builder::CreateRet(llvm::Value *value)
{
    Assert(value != nullptr, "value must not be null");

    return m_LLVMBuilder.CreateRet(value);
}

llvm::BasicBlock *llove::Builder::CreateBlock(const std::string &name)
{
    return llvm::BasicBlock::Create(m_LLVMContext, name, nullptr);
}

llvm::BasicBlock *llove::Builder::CreateBlock(const std::string &name, llvm::Function *parent)
{
    Assert(parent != nullptr, "parent must not be null");

    return llvm::BasicBlock::Create(m_LLVMContext, name, parent);
}
