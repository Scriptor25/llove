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
    struct GenericFunction final
    {
        bool Interface = false;

        ClassType::Ptr Class;
        bool Mutable = false;
        bool Expose = false;

        std::string Name;
        std::vector<Parameter> Parameters;
        bool VarArg = false;
        Field Result;

        Statement *Content = nullptr;
    };

    struct DestructorReference final
    {
        llvm::Value *Self = nullptr;
        llvm::FunctionCallee Callee;
    };

    struct Frame
    {
        std::vector<DestructorReference> Destructors;
        std::map<std::string, std::pair<bool, ValuePtr>> Values;
    };

    class Builder
    {
    public:
        explicit Builder(Context &types);

        Context &GetTypes() const;

        static std::string Mangle(
            bool interface,
            const ClassType::Ptr &class_type,
            bool mutable_,
            const std::string &name,
            const std::vector<Parameter> &parameters,
            bool vararg,
            const Field &result);

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

        llvm::Value *CreateAlloca(const TypePtr &type, llvm::Function *parent = nullptr);

        llvm::Value *CreateLoad(llvm::Value *pointer, const TypePtr &type);
        llvm::Value *CreateStore(llvm::Value *pointer, llvm::Value *value, bool volatile_ = false);
        llvm::Value *CreateStore(llvm::Value *pointer, const ValuePtr &value, bool volatile_ = false);

        void CreateRetVoid();
        void CreateRet(llvm::Value *value);

        llvm::Value *CreateCall(
            const FunctionType::Ptr &type,
            llvm::Value *callee,
            const std::vector<llvm::Value *> &arguments);
        llvm::Value *CreateCall(llvm::FunctionCallee callee, const std::vector<llvm::Value *> &arguments);
        ValuePtr CreateCall(
            const FunctionType::Ptr &type,
            llvm::Value *callee,
            std::vector<ValuePtr> arguments,
            ValuePtr self);

        llvm::Value *CreateInsertValue(llvm::Value *aggregate, llvm::Value *value, unsigned index);
        llvm::Value *CreateExtractValue(llvm::Value *aggregate, unsigned index);
        llvm::Value *CreateExtractValue(const ValuePtr &aggregate, unsigned index);

        llvm::Value *CreatePointerOffset(llvm::Type *element_type, llvm::Value *pointer, unsigned offset);
        ValuePtr CreatePointerOffset(const ValuePtr &pointer, const ValuePtr &offset);
        ValuePtr CreatePointerDifference(const ValuePtr &begin, const ValuePtr &end);

        ValuePtr CreatePointerElement(const ValuePtr &pointer, const ValuePtr &index);
        ValuePtr CreateArrayElement(ValuePtr array, const ValuePtr &index);
        llvm::Value *CreateArrayGEP(const TypePtr &type, llvm::Value *pointer, unsigned index);
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

        llvm::Value *CreatePCmpEQ(llvm::Value *left, llvm::Value *right);
        ValuePtr CreatePCmpEQ(const ValuePtr &left, const ValuePtr &right);
        llvm::Value *CreatePCmpNE(llvm::Value *left, llvm::Value *right);
        ValuePtr CreatePCmpNE(const ValuePtr &left, const ValuePtr &right);

        ValuePtr CreateNeg(const ValuePtr &operand);
        ValuePtr CreateFNeg(const ValuePtr &operand);

        ValuePtr CreateNot(const ValuePtr &operand);
        ValuePtr CreateInv(const ValuePtr &operand);

        llvm::Value *CreateIncrement(const TypePtr &type, llvm::Value *value);
        llvm::Value *CreateCompareNE(TypePtr type, llvm::Value * left, llvm::Value *right);

        void CreateBranch(llvm::BasicBlock *block);
        void CreateBranch(llvm::Value *condition, llvm::BasicBlock *then, llvm::BasicBlock *else_);
        void CreateBranch(const ValuePtr &condition, llvm::BasicBlock *then, llvm::BasicBlock *else_);

        void SetInsertPoint(llvm::BasicBlock *block);
        void ClearInsertPoint();
        bool NoTerminator() const;

        llvm::Function *GetParent() const;
        const Field &GetResult() const;

        llvm::Function *GetOrCreateFunction(const std::string &name, const FunctionType::Ptr &type, bool external);
        llvm::BasicBlock *CreateBlock(const std::string &name, llvm::Function *parent = nullptr);

        FunctionReference &PushFunction(bool expose, std::string name, FunctionType::Ptr type, llvm::Function *callee);
        std::vector<FunctionReference> GetFunctions(const std::string &name);
        std::vector<FunctionReference> GetFunctions(const std::string &name, const Field &self);

        bool HasFunction(
            const std::vector<FunctionReference> &functions,
            const std::vector<Field> &arguments,
            bool has_self,
            const Field &self = {}) const;
        const FunctionReference *FindFunction(
            const std::vector<FunctionReference> &functions,
            const std::vector<Field> &arguments,
            bool has_self,
            const Field &self = {}) const;
        const ClassFunctionReference *FindFunction(
            const std::vector<ClassFunctionReference> &functions,
            const std::vector<Field> &arguments,
            const ClassType::Ptr &class_type,
            const Field &self) const;

        Operator<1>::Ptr FindOperator(const std::string &operator_, const Field &operand, bool suffix);
        Operator<2>::Ptr FindOperator(const std::string &operator_, const Field &left, const Field &right);

        void PushFrame();
        void PopFrame();

        void SetValue(const std::string &name, ValuePtr value);
        bool HasValue(const std::string &name) const;
        ValuePtr GetValue(const std::string &name) const;

        void PushDestructor(llvm::Value *self, llvm::FunctionCallee callee);
        void PopDestructor(const llvm::Value *self);

        ValuePtr CreateCast(ValuePtr value, TypePtr dst);
        bool IsCastable(const Field &src, const Field &dst) const;

        llvm::Value *CreateGlobalString(const std::string &value);

        FunctionReference &GenFunction(const GenericFunction &fn);
        void GenParameters(llvm::Function *function, const std::vector<Parameter> &parameters, const Field &self = {});

        void Seal(const std::string &filename);

    private:
        Context &m_Types;

        llvm::LLVMContext m_Context;
        llvm::IRBuilder<> m_Builder;
        llvm::Module m_Module;

        std::vector<FunctionReference> m_Functions;

        llvm::Function *m_Parent;
        Field m_Result;
        std::vector<Frame> m_Stack;
    };
}
