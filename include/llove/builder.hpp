#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/location.hpp>
#include <llove/operator.hpp>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Target/TargetOptions.h>

namespace llove
{
    struct FunctionInfo final
    {
        Location Loc;

        bool Interface = false;
        bool Implicit = false;

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

    struct Frame final
    {
        llvm::DIScope *Scope = nullptr;
        std::map<llvm::Value *, llvm::FunctionCallee> Destructors;
        std::map<std::string, ValuePtr> Values;
    };

    struct SealInfo final
    {
        bool Print;
        std::ostream *PrintStream;
        std::ostream *OutputStream;
        llvm::CodeGenFileType Format;
        std::string Triple;
        std::string CPU;
        std::vector<std::string> Features;
        llvm::TargetOptions Options;
        llvm::Reloc::Model Relocation;
        llvm::OptimizationLevel Level;
    };

    class Builder
    {
    public:
        explicit Builder(Context &types, const std::filesystem::path &filepath);

        Context &GetTypes() const;

        llvm::DIScope *GetDbgScope() const;
        llvm::DIFile *GetDbgFile() const;

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

        llvm::DIType *GetDbgVoidType();
        llvm::DIType *GetDbgIntType(bool sign, unsigned bits);
        llvm::DIType *GetDbgFltType(unsigned bits);
        llvm::DIType *GetDbgPointerType();
        llvm::DIType *GetDbgPointerType(llvm::DIType *base);
        llvm::DIType *GetDbgArrayType(llvm::DIType *base, unsigned size);
        llvm::DIType *GetDbgStructType(const std::vector<llvm::Metadata *> &fields, unsigned size);
        llvm::DIType *GetDbgFieldType(const std::string &name, llvm::DIType *type, unsigned size, unsigned offset);
        llvm::DIType *GetDbgClassType(const std::string &name);
        llvm::DIType *GetDbgClassType(const std::string &name, const std::vector<llvm::Metadata *> &fields);
        llvm::DISubroutineType *GetDbgFunctionType(
            const std::vector<llvm::Metadata *> &parameters,
            llvm::DIType *result);

        void CreateDbgParameter(const std::string &name, unsigned index, const ValuePtr &value);
        void CreateDbgVariable(const std::string &name, const ValuePtr &value);

        void EmitLoc();
        void EmitLoc(const Location &loc);
        void EmitLoc(const GlobalPtr &ptr);
        void EmitLoc(const StatementPtr &ptr);

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
        ValuePtr CreatePointerOffset(const ValuePtr &pointer, unsigned offset);
        ValuePtr CreatePointerOffset(const ValuePtr &pointer, const ValuePtr &offset);
        ValuePtr CreatePointerDifference(const ValuePtr &begin, const ValuePtr &end);

        ValuePtr CreatePointerElement(const ValuePtr &pointer, const ValuePtr &index);
        ValuePtr CreateArrayElement(const ValuePtr &array, const ValuePtr &index);
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

        void CreateBranch(llvm::BasicBlock *block);
        void CreateBranch(llvm::Value *condition, llvm::BasicBlock *then, llvm::BasicBlock *else_);
        void CreateBranch(const ValuePtr &condition, llvm::BasicBlock *then, llvm::BasicBlock *else_);

        void SetInsertPoint(llvm::BasicBlock *block);
        void ClearInsertPoint();
        bool NoTerminator() const;

        llvm::Function *GetParent() const;
        const Field &GetResult() const;
        ClassType::Ptr GetClass() const;

        llvm::Function *GetOrCreateFunction(const std::string &name, const FunctionType::Ptr &type, bool external);
        llvm::BasicBlock *CreateBlock(const std::string &name, llvm::Function *parent = nullptr);

        FunctionReference &PushFunction(
            bool expose,
            bool implicit,
            std::string name,
            FunctionType::Ptr type,
            llvm::Function *callee);
        [[nodiscard]] std::vector<FunctionReference> GetFunctions(const std::string &name) const;
        [[nodiscard]] std::vector<FunctionReference> GetFunctions(const std::string &name, const Field &self) const;

        bool HasFunction(
            const std::vector<FunctionReference> &functions,
            const std::vector<Field> &arguments,
            bool has_self,
            const Field &self = {}) const;
        std::optional<FunctionReference> FindFunction(
            const std::vector<FunctionReference> &functions,
            const std::vector<Field> &arguments,
            const Field &self = {}) const;
        std::optional<FunctionReference> FindFunction(
            const std::vector<ClassFunctionReference> &functions,
            const std::vector<Field> &arguments,
            const ClassType::Ptr &class_type,
            const Field &self,
            bool implicit);

        Operator<1>::Ptr FindOperator(const std::string &operator_, const Field &operand, bool suffix);
        Operator<2>::Ptr FindOperator(const std::string &operator_, const Field &left, const Field &right);

        void PushFrame(llvm::DIScope *scope = nullptr);
        void PopFrame();

        void SetValue(const std::string &name, ValuePtr value);
        bool HasValue(const std::string &name) const;
        ValuePtr GetValue(const std::string &name) const;

        void PushDestructor(llvm::Value *self, llvm::FunctionCallee callee);
        void CallDestructors(const std::set<llvm::Value *> &mask, bool propagate);

        ValuePtr CreateCast(ValuePtr value, TypePtr dst, bool implicit);
        bool IsCastable(const Field &src, const Field &dst, bool implicit) const;

        llvm::Value *CreateGlobalString(const std::string &value);

        FunctionReference &GenFunction(const FunctionInfo &fn);
        void GenParameters(llvm::Function *function, const std::vector<Parameter> &parameters, const Field &self = {});

        void Seal(const SealInfo &info);

    private:
        Context &m_Types;

        llvm::LLVMContext m_Context;
        llvm::IRBuilder<> m_Builder;
        llvm::Module m_Module;

        llvm::DIBuilder m_DIBuilder;
        llvm::DICompileUnit *m_CompileUnit;

        std::vector<FunctionReference> m_Functions;

        llvm::Function *m_Parent = nullptr;
        ClassType::Ptr m_Class;
        Field m_Result;
        std::vector<Frame> m_Stack;
    };
}
