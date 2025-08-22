#include <llove/builder.hpp>

void llove::Builder::SetCurrentDebugLocation(llvm::DebugLoc loc)
{
    m_LLVMBuilder.SetCurrentDebugLocation(std::move(loc));
}

void llove::Builder::SetInsertPoint(llvm::BasicBlock *block)
{
    Assert(block != nullptr, "block must not be null");

    m_LLVMBuilder.SetInsertPoint(block);
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

llvm::Value *llove::Builder::CreateNotNull(llvm::Value *value)
{
    Assert(value != nullptr, "value must not be null");

    return m_LLVMBuilder.CreateIsNotNull(value);
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

llvm::Constant *llove::Builder::CreateGlobalString(const std::string &value, const std::string &name)
{
    return m_LLVMBuilder.CreateGlobalStringPtr(value, name, 0, &m_LLVMModule);
}
