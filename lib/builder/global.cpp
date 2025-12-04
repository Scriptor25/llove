#include <llove/builder.hpp>
#include <llvm/IR/Constant.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>

llvm::Value* llove::Builder::CreateGlobal(
    std::string name,
    llvm::Type* type,
    bool is_external,
    llvm::GlobalValue::LinkageTypes linkage,
    llvm::Constant* initializer)
{
    return new llvm::GlobalVariable(m_LLVMModule, type, false, linkage, initializer, name, nullptr, llvm::GlobalValue::NotThreadLocal, std::nullopt, is_external);
}
