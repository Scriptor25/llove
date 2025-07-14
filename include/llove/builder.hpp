#pragma once

#include <map>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/operator.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace llove
{
    struct GenericFunction
    {
        bool Interface = false;

        std::string ClassName;
        bool Mutable = false;
        bool Expose = false;

        std::string Name;
        std::vector<Parameter> Parameters;
        bool VarArg = false;
        Field Result;

        Statement *Content = nullptr;
    };

    struct Frame
    {
        Field Result;
        std::map<std::string, ValuePtr> Values;
    };

    class Builder
    {
    public:
        explicit Builder(Context &types);

        Context &GetTypes() const;

        std::string Mangle(
            bool interface,
            const std::string &class_name,
            bool mutable_,
            const std::string &name,
            const std::vector<Parameter> &parameters,
            bool vararg,
            const Field &result) const;

        llvm::Type *GetVoidType();
        llvm::IntegerType *GetIntType(unsigned bits);
        llvm::Type *GetFltType(unsigned bits);
        llvm::ArrayType *GetArrayType(llvm::Type *base, unsigned size);
        llvm::PointerType *GetPointerType();
        llvm::PointerType *GetPointerType(llvm::Type *base);
        llvm::StructType *GetStructType(const std::vector<llvm::Type *> &fields, bool packed);
        llvm::FunctionType *GetFunctionType(
            llvm::Type *result,
            const std::vector<llvm::Type *> &parameters,
            bool vararg);

        llvm::StructType *GetNamedStructType(const std::string &name);
        llvm::StructType *GetOrCreateNamedStructType(const std::string &name);
        llvm::StructType *GetOrCreateNamedStructType(
            const std::string &name,
            const std::vector<llvm::Type *> &fields,
            bool packed);

        llvm::Value *CreateAlloca(llvm::Function *parent, const TypePtr &type);

        llvm::Value *CreateLoad(llvm::Value *pointer, const TypePtr &type);
        llvm::Value *CreateStore(llvm::Value *pointer, llvm::Value *value, bool volatile_ = false);

        void CreateRetVoid();
        void CreateRet(llvm::Value *value);

        llvm::Value *CreateCall(
            const FunctionType::Ptr &type,
            llvm::Value *callee,
            const std::vector<llvm::Value *> &arguments);
        llvm::Value *CreateCall(llvm::FunctionCallee callee, const std::vector<llvm::Value *> &arguments);

        llvm::Value *CreateInsertValue(llvm::Value *aggregate, llvm::Value *value, unsigned index);
        llvm::Value *CreateExtractValue(llvm::Value *aggregate, unsigned index);

        ValuePtr CreatePointerOffset(const ValuePtr &pointer, const ValuePtr &offset);
        ValuePtr CreatePointerDifference(const ValuePtr &begin, const ValuePtr &end);

        ValuePtr CreatePointerElement(const ValuePtr &pointer, const ValuePtr &index);
        ValuePtr CreateArrayElement(ValuePtr array, const ValuePtr &index);
        llvm::Value *CreateStructGEP(const TypePtr &type, llvm::Value *pointer, unsigned index);

        ValuePtr CreateAdd(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateSub(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateMul(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateDiv(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateRem(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreateAnd(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateOr(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateXor(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreateCmpEQ(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateCmpNE(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateCmpLT(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateCmpGT(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateCmpLE(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateCmpGE(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreateFAdd(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFSub(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFMul(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFDiv(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFRem(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreateFCmpEQ(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFCmpNE(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFCmpLT(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFCmpGT(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFCmpLE(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreateFCmpGE(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreatePCmpEQ(const ValuePtr &left, const ValuePtr &right);
        ValuePtr CreatePCmpNE(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreateNeg(const ValuePtr &operand);
        ValuePtr CreateFNeg(const ValuePtr &operand);

        ValuePtr CreateNot(const ValuePtr &operand);
        ValuePtr CreateInv(const ValuePtr &operand);

        void CreateBranch(llvm::BasicBlock *block);
        void CreateBranch(const ValuePtr &condition, llvm::BasicBlock *then, llvm::BasicBlock *else_);

        llvm::BasicBlock *GetInsertBlock() const;
        void SetInsertPoint(llvm::BasicBlock *block);
        void SetInsertPointPastAllocas(llvm::Function *parent);
        void ClearInsertPoint();

        llvm::Function *GetParent() const;

        llvm::Function *GetOrCreateFunction(const std::string &name, const FunctionType::Ptr &type, bool external);
        llvm::BasicBlock *CreateBlock(const std::string &name, llvm::Function *parent = nullptr);

        void AddFunction(bool expose, std::string name, FunctionType::Ptr type, llvm::Function *callee);
        std::vector<FunctionReference> GetFunctions(const std::string &name);
        std::vector<FunctionReference> GetFunctions(const std::string &name, const Field &self);

        Operator<1>::Ptr GetOperator(const std::string &operator_, const Field &operand, bool suffix);
        Operator<2>::Ptr GetOperator(const std::string &operator_, const Field &left, const Field &right);

        void StackPush(const Field &result = {});
        void StackPop();
        void SetValue(const std::string &name, ValuePtr value);
        ValuePtr GetValue(const std::string &name) const;
        Field GetResult();

        ValuePtr CreateCast(ValuePtr value, TypePtr type);
        bool IsCastable(bool mutable_, const TypePtr &value_type, const TypePtr &type);

        llvm::Value *CreateGlobalString(const std::string &value);

        llvm::FunctionCallee GenFunction(const GenericFunction &fn);
        void GenParameters(llvm::Function *parent, const std::vector<Parameter> &parameters, const Field &self = {});

        void Gen(const std::string &filename);

    private:
        Context &m_Types;

        llvm::LLVMContext m_Context;
        llvm::IRBuilder<> m_Builder;
        llvm::Module m_Module;

        std::vector<FunctionReference> m_Functions;
        std::vector<Frame> m_Stack;
    };
}
