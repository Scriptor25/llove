#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <llove/debug.hpp>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/location.hpp>
#include <llove/operator.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

namespace llove
{
    struct FunctionInfo final
    {
        Location Loc;

        bool Register = true;
        bool Export = false;

        bool Interface = false;
        bool Implicit = false;

        ClassType::Ptr Class;
        bool Mutable = false;
        bool Expose = false;

        std::string Name;
        std::vector<Parameter> Parameters;
        std::pair<bool, std::string> Variadic;
        Field Result;

        Statement *Content = nullptr;
    };

    struct Frame final
    {
        llvm::BasicBlock *Head = nullptr;
        llvm::BasicBlock *Tail = nullptr;

        std::map<std::string, ValuePtr> Values;
        std::vector<std::pair<llvm::Value *, std::function<void()>>> Deferred;
    };

    struct Machine final
    {
        std::string Triple;
        std::string CPU;
        std::vector<std::string> Features;
        llvm::TargetOptions Options;
        llvm::Reloc::Model Relocation;
    };

    class Builder
    {
    public:
        Builder(
            Context &context,
            const Machine &machine,
            bool debug,
            bool optimized,
            bool profiling,
            llvm::DICompileUnit::DebugEmissionKind emission,
            const std::filesystem::path &source_path,
            const std::string &command_line);
        Builder(
            Context &context,
            const Machine &machine,
            bool debug,
            bool optimized,
            bool profiling,
            llvm::DICompileUnit::DebugEmissionKind emission,
            const std::filesystem::path &source_path,
            const std::string &command_line,
            const std::string &module_id);

        Context &GetContext() const;

        bool IsDebug() const;
        DebugBuilder &GetDebug();

        void EmitLoc(const Location &loc);

        llvm::LLVMContext &GetLLVMContext();

        const llvm::DataLayout &GetDataLayout() const;

        static std::string Mangle(
            bool interface,
            const ClassType::Ptr &class_type,
            bool mutable_,
            const std::string &name,
            const std::vector<Parameter> &parameters,
            bool variadic,
            const Field &result);

        llvm::Type *GetVoidType();
        llvm::IntegerType *GetIntegerType(unsigned bits);
        llvm::IntegerType *GetPointerSizeType();
        llvm::Type *GetFloatType(unsigned bits);
        llvm::ArrayType *GetArrayType(llvm::Type *base, unsigned size);
        llvm::PointerType *GetPointerType();
        llvm::FunctionType *GetFunctionType(llvm::Type *result, const std::vector<llvm::Type *> &parameters);
        llvm::StructType *GetStructType(const std::vector<llvm::Type *> &fields, bool packed);
        llvm::StructType *GetNamedStructType(const std::string &name);
        llvm::StructType *GetOrCreateNamedStructType(const std::string &name);
        llvm::StructType *GetOrCreateNamedStructType(
            const std::string &name,
            const std::vector<llvm::Type *> &fields,
            bool packed);
        llvm::StructType *GetVariadicType();

#pragma region WRAPPER

        void SetCurrentDebugLocation(llvm::DebugLoc loc);

        void SetInsertPoint(llvm::BasicBlock *block);
        void SetInsertPoint(llvm::Instruction *instruction);
        void ClearInsertionPoint();
        llvm::BasicBlock *GetInsertBlock() const;

        llvm::AllocaInst *CreateAlloca(
            llvm::Type *type,
            llvm::Function *parent = nullptr,
            llvm::Value *array_size = nullptr,
            const std::string &name = {});

        llvm::LoadInst *CreateLoad(
            llvm::Type *type,
            llvm::Value *pointer,
            bool is_volatile = false,
            const std::string &name = {});
        llvm::StoreInst *CreateStore(llvm::Value *value, llvm::Value *pointer, bool is_volatile = false);

        llvm::Value *CreateArrayGEP(
            llvm::Type *type,
            llvm::Value *pointer,
            unsigned index,
            const std::string &name = {});
        llvm::Value *CreateStructGEP(
            llvm::Type *type,
            llvm::Value *pointer,
            unsigned index,
            const std::string &name = {});
        llvm::Value *CreateGEP(llvm::Type *type, llvm::Value *pointer, unsigned index, const std::string &name = {});
        llvm::Value *CreateGEP(
            llvm::Type *type,
            llvm::Value *pointer,
            llvm::Value *index,
            const std::string &name = {},
            bool is_in_bounds = false);

        llvm::Value *CreateExtractValue(llvm::Value *aggregate, unsigned index, const std::string &name = {});
        llvm::Value *CreateInsertValue(
            llvm::Value *aggregate,
            llvm::Value *value,
            unsigned index,
            const std::string &name = {});

        llvm::Value *CreateAdd(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateSub(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateMul(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateDiv(
            bool is_signed,
            llvm::Value *left,
            llvm::Value *right,
            const std::string &name = {},
            bool is_exact = false);
        llvm::Value *CreateRem(
            bool is_signed,
            llvm::Value *left,
            llvm::Value *right,
            const std::string &name = {});

        llvm::Value *CreateAnd(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateOr(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateXor(llvm::Value *left, llvm::Value *right, const std::string &name = {});

        llvm::Value *CreateFAdd(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFSub(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFMul(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFDiv(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFRem(llvm::Value *left, llvm::Value *right, const std::string &name = {});

        llvm::Value *CreateNeg(llvm::Value *value, const std::string &name = {});
        llvm::Value *CreateFNeg(llvm::Value *value, const std::string &name = {});
        llvm::Value *CreateNot(llvm::Value *value, const std::string &name = {});

        llvm::Value *CreateCmpEQ(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateCmpNE(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateCmpLT(bool is_signed, llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateCmpLE(bool is_signed, llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateCmpGT(bool is_signed, llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateCmpGE(bool is_signed, llvm::Value *left, llvm::Value *right, const std::string &name = {});

        llvm::Value *CreateFCmpEQ(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFCmpNE(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFCmpLT(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFCmpLE(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFCmpGT(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreateFCmpGE(llvm::Value *left, llvm::Value *right, const std::string &name = {});

        llvm::Value *CreatePCmpEQ(llvm::Value *left, llvm::Value *right, const std::string &name = {});
        llvm::Value *CreatePCmpNE(llvm::Value *left, llvm::Value *right, const std::string &name = {});

        llvm::Value *CreatePtrDiff(
            llvm::Type *element_type,
            llvm::Value *left,
            llvm::Value *right,
            const std::string &name = {});

        llvm::Value *CreateIsNotNull(llvm::Value *value, const std::string &name = {});
        llvm::Value *CreateIsNull(llvm::Value *value, const std::string &name = {});

        llvm::BranchInst *CreateBranch(llvm::BasicBlock *block);
        llvm::BranchInst *CreateBranch(
            llvm::Value *condition,
            llvm::BasicBlock *true_block,
            llvm::BasicBlock *false_block);
        llvm::SwitchInst *CreateSwitch(
            llvm::Value *condition,
            llvm::BasicBlock *default_block,
            const std::map<llvm::ConstantInt *, llvm::BasicBlock *> &cases);
        llvm::PHINode *CreatePHI(llvm::Type *type, const std::map<llvm::BasicBlock *, llvm::Value *> &operands);

        llvm::ReturnInst *CreateRetVoid();
        llvm::ReturnInst *CreateRet(llvm::Value *value);

        llvm::BasicBlock *CreateBlock(const std::string &name = {});
        llvm::BasicBlock *CreateBlock(const std::string &name, llvm::Function *parent);

        llvm::Constant *CreateGlobalString(const std::string &value, const std::string &name = {});

#pragma endregion

        ValuePtr CreateCall(const FunctionReference &function, std::vector<ValuePtr> arguments, ValuePtr self);
        ValuePtr CreateCall(const ValuePtr &callee);

        ValuePtr GetPointerElement(const ValuePtr &pointer, const ValuePtr &index);
        ValuePtr GetArrayElement(const ValuePtr &array, const ValuePtr &index);

        llvm::Function *GetParent() const;
        Field GetResult() const;
        ClassType::Ptr GetClass() const;

        llvm::BasicBlock *GetHead() const;
        llvm::BasicBlock *GetTail() const;

        llvm::Function *GetOrCreateFunction(const std::string &name, const FunctionType::Ptr &type, bool external);

        FunctionReference &PushFunction(
            bool expose,
            bool implicit,
            std::string name,
            FunctionType::Ptr type,
            llvm::Function *callee);
        [[nodiscard]] std::vector<FunctionReference> GetFunctions(
            const std::string &name,
            const std::optional<Field> &self = std::nullopt) const;

        bool HasFunction(
            const std::vector<FunctionReference> &functions,
            const std::vector<Field> &arguments,
            const std::optional<Field> &self = std::nullopt) const;
        std::optional<FunctionReference> FindFunction(
            const std::vector<FunctionReference> &functions,
            const std::vector<Field> &arguments,
            const std::optional<Field> &self = std::nullopt) const;
        std::optional<FunctionReference> FindFunction(
            const std::vector<ClassFunctionReference> &functions,
            const std::vector<Field> &arguments,
            const ClassType::Ptr &class_type,
            const Field &self,
            bool implicit);

        Operator<1>::Ptr FindOperator(const std::string &operator_, const Field &operand, bool suffix);
        Operator<2>::Ptr FindOperator(const std::string &operator_, const Field &left, const Field &right);

        void PushFrame(
            const std::optional<Location> &loc = std::nullopt,
            llvm::BasicBlock *head = nullptr,
            llvm::BasicBlock *tail = nullptr);
        void PopFrame();

        void SetValue(const std::string &name, ValuePtr value);
        bool HasValue(const std::string &name) const;
        ValuePtr GetValue(const std::string &name) const;

        void DeferAction(llvm::Value *key, std::function<void()> action);
        void PushDestructor(llvm::Value *self, const FunctionReference &callee);
        void CallDeferred(const std::set<llvm::Value *> &mask, bool propagate);

        ValuePtr CreateCast(ValuePtr value, TypePtr dst, bool implicit);
        bool IsCastable(const Field &src, const Field &dst, bool implicit) const;

        FunctionReference GenFunction(const FunctionInfo &fn);
        void GenParameters(
            llvm::Function *parent,
            const std::vector<Parameter> &parameters,
            const std::pair<bool, std::string> &variadic,
            const std::optional<Field> &self = std::nullopt);

        void Seal(
            bool print,
            std::ostream &print_stream,
            std::ostream &output_stream,
            llvm::CodeGenFileType code_gen_type,
            llvm::OptimizationLevel optimization_level);

    private:
        Context &m_Context;

        llvm::LLVMContext m_LLVMContext;
        llvm::IRBuilder<> m_LLVMBuilder;
        llvm::Module m_LLVMModule;

        std::unique_ptr<llvm::TargetMachine> m_TargetMachine;

        bool m_Debug;
        DebugBuilder m_DebugBuilder;

        std::vector<FunctionReference> m_Functions;

        llvm::Function *m_Parent = nullptr;
        ClassType::Ptr m_Class;
        Field m_Result;
        std::vector<Frame> m_Stack;
    };
}
