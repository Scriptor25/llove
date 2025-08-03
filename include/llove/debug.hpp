#pragma once

#include <filesystem>
#include <llove/forward.hpp>
#include <llove/type.hpp>
#include <llvm/IR/DIBuilder.h>

namespace llove
{
    class DebugBuilder final
    {
    public:
        DebugBuilder(
            bool enable,
            llvm::Module &module,
            const std::filesystem::path &source_path,
            const std::filesystem::path &debug_path,
            bool optimized,
            bool profiling,
            const std::string &command_line,
            llvm::DICompileUnit::DebugEmissionKind emission);

        llvm::DIType *GetVoidType() const;
        llvm::DIType *GetIntegerType(bool sign, unsigned bits) const;
        llvm::DIType *GetFloatType(unsigned bits) const;
        llvm::DIType *GetPointerType() const;
        llvm::DIType *GetPointerType(llvm::DIType *base) const;
        llvm::DIType *GetArrayType(llvm::DIType *base, unsigned size) const;
        llvm::DIType *GetStructType(const std::vector<llvm::Metadata *> &fields, unsigned size) const;
        llvm::DIType *GetFieldType(const std::string &name, llvm::DIType *type, unsigned size, unsigned offset) const;
        llvm::DIType *GetClassType(const std::string &name) const;
        llvm::DIType *GetClassType(const std::string &name, const std::vector<llvm::Metadata *> &fields, unsigned size) const;
        llvm::DISubroutineType *GetFunctionType(
            const std::vector<llvm::Metadata *> &parameters,
            llvm::DIType *result) const;

        void CreateParameter(Builder &builder, const std::string &name, unsigned index, const ValuePtr &value) const;
        void CreateVariable(Builder &builder, const std::string &name, const ValuePtr &value) const;

        void EmitLoc(Builder &builder) const;
        void EmitLoc(Builder &builder, const Location &loc) const;
        void EmitLoc(Builder &builder, const GlobalPtr &ptr) const;
        void EmitLoc(Builder &builder, const StatementPtr &ptr) const;

        void EndModule() const;

        void BeginFunction(
            Builder &builder,
            const std::string &name,
            const Location &loc,
            const FunctionType::Ptr &function_type,
            const std::string &mangled_name,
            llvm::Function *function);
        void EndFunction();

    protected:
        [[nodiscard]] llvm::DIScope *GetScope() const;

    private:
        bool m_Strip;

        std::unique_ptr<llvm::DIBuilder> m_DIBuilder;
        llvm::DICompileUnit *m_CompileUnit = nullptr;

        std::vector<llvm::DISubprogram *> m_Stack;
    };
}
