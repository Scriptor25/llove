#pragma once

#include <llove/forward.hpp>
#include <llove/type.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace llove
{
    class Builder
    {
    public:
        explicit Builder(Context &types);

        Context &GetTypes() const;

        llvm::Type *GetVoidType();
        llvm::IntegerType *GetIntType(unsigned bits);
        llvm::Type *GetFltType(unsigned bits);
        llvm::ArrayType *GetArrayType(llvm::Type *base, unsigned size);
        llvm::PointerType *GetPtrType(llvm::Type *base);
        llvm::StructType *GetStructType(const std::vector<llvm::Type *> &fields, bool packed);
        llvm::FunctionType *GetFunctionType(
            llvm::Type *result,
            const std::vector<llvm::Type *> &parameters,
            bool vararg);

        llvm::StructType *GetNamedStructType(const std::string_view &name);
        llvm::StructType *GetOrCreateNamedStructType(const std::string_view &name);
        llvm::StructType *GetOrCreateNamedStructType(
            const std::string_view &name,
            const std::vector<llvm::Type *> &fields,
            bool packed);

        llvm::Value *CreateLoad(llvm::Value *pointer, const TypePtr &type);
        llvm::Value *CreateStore(llvm::Value *pointer, llvm::Value *value, bool volatile_);

        llvm::Function *CreateFunction(const std::string_view &name, const FunctionType::Ptr &type, bool external);

    private:
        Context &m_Types;

        llvm::LLVMContext m_Context;
        llvm::IRBuilder<> m_Builder;
        llvm::Module m_Module;
    };
}
