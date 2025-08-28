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
            bool optimized,
            bool profiling,
            const std::string &command_line,
            llvm::DICompileUnit::DebugEmissionKind emission);

        [[nodiscard]] llvm::DIType *GetVoidType() const;
        [[nodiscard]] llvm::DIType *GetIntegerType(bool sign, unsigned bits) const;
        [[nodiscard]] llvm::DIType *GetFloatType(unsigned bits) const;
        [[nodiscard]] llvm::DIType *GetPointerType() const;
        [[nodiscard]] llvm::DIType *GetPointerType(llvm::DIType *base) const;
        [[nodiscard]] llvm::DIType *GetArrayType(llvm::DIType *base, unsigned size) const;
        [[nodiscard]] llvm::DIType *GetStructType(const std::vector<llvm::Metadata *> &elements, unsigned size) const;
        [[nodiscard]] llvm::DIType *GetVariadicType() const;
        [[nodiscard]] llvm::DIType *GetFieldType(
            const std::string &name,
            llvm::DIType *type,
            unsigned size,
            unsigned offset) const;
        [[nodiscard]] llvm::DIType *GetClassType(const std::string &name) const;
        [[nodiscard]] llvm::DIType *GetClassType(
            const std::string &name,
            llvm::DIType *base,
            const std::vector<llvm::Metadata *> &elements,
            unsigned size) const;
        [[nodiscard]] llvm::DISubroutineType *GetFunctionType(
            llvm::DIType *self,
            const std::vector<llvm::Metadata *> &parameters,
            bool variadic,
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

        void PushFrame(const std::optional<Location> &loc);
        void PopFrame();

    protected:
        [[nodiscard]] llvm::DIScope *GetScope() const;

    private:
        bool m_Strip;

        std::unique_ptr<llvm::DIBuilder> m_DIBuilder;
        llvm::DICompileUnit *m_CompileUnit = nullptr;

        std::vector<llvm::DISubprogram *> m_Subprograms;
        std::vector<llvm::DIScope *> m_Scopes;
    };
}
