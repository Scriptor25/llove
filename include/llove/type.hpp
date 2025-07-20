#pragma once

#include <format>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <llove/class.hpp>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace llove
{
    enum TypeId
    {
        TypeId_Void,
        TypeId_Integer,
        TypeId_Float,
        TypeId_Pointer,
        TypeId_Array,
        TypeId_Struct,
        TypeId_Range,
        TypeId_Class,
        TypeId_Function,
    };

    class Type
    {
    public:
        virtual ~Type() = default;
        [[nodiscard]] virtual TypeId GetId() const = 0;
        virtual llvm::Type *Gen(Builder &builder) const = 0;
        [[nodiscard]] virtual std::string Mangle() const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    class VoidType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VoidType>;

        explicit VoidType() = default;

        [[nodiscard]] TypeId GetId() const override;
        llvm::Type *Gen(Builder &builder) const override;

        /**
         * @return v
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;
    };

    class IntegerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<IntegerType>;

        explicit IntegerType(bool sign, unsigned bits);

        [[nodiscard]] bool IsSigned() const;
        [[nodiscard]] unsigned GetBits() const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::IntegerType *Gen(Builder &builder) const override;

        /**
         * @return <sign?i:u><bits>_
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        bool m_Sign;
        unsigned m_Bits;
    };

    class FloatType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FloatType>;

        explicit FloatType(unsigned bits);

        [[nodiscard]] unsigned GetBits() const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::Type *Gen(Builder &builder) const override;

        /**
         * @return f<bits>_
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        unsigned m_Bits;
    };

    class PointerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<PointerType>;

        explicit PointerType(TypePtr base, bool mutable_);

        [[nodiscard]] TypePtr GetBase() const;
        [[nodiscard]] bool IsMutable() const;
        [[nodiscard]] bool IsOpaque() const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::PointerType *Gen(Builder &builder) const override;

        /**
         * @return p<mutable?m:i><base>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Base;
        bool m_Mutable;
    };

    class ArrayType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ArrayType>;

        explicit ArrayType(TypePtr base, unsigned size);

        [[nodiscard]] TypePtr GetBase() const;
        [[nodiscard]] unsigned GetSize() const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::ArrayType *Gen(Builder &builder) const override;

        /**
         * @return a<size>_<base>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Base;
        unsigned m_Size;
    };

    class StructType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<StructType>;

        explicit StructType(std::vector<Parameter> fields);

        [[nodiscard]] bool HasField(const std::string &name) const;
        [[nodiscard]] unsigned GetFieldIndex(const std::string &name) const;
        [[nodiscard]] unsigned GetFieldCount() const;
        [[nodiscard]] const Field &GetField(unsigned index) const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return s<length>_<fields...>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<Parameter> m_Fields;
    };

    class RangeType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<RangeType>;

        explicit RangeType(TypePtr entry);

        [[nodiscard]] TypePtr GetEntry() const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return r<entry>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Entry;
    };

    class ClassType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ClassType>;

        explicit ClassType(std::string name);
        explicit ClassType(
            std::string name,
            std::vector<ClassFieldReference> fields,
            std::vector<ClassFunctionReference> functions);

        [[nodiscard]] const std::string &GetName() const;
        [[nodiscard]] bool IsOpaque() const;

        [[nodiscard]] unsigned GetFieldIndex(const std::string &name) const;

        [[nodiscard]] unsigned GetFieldCount() const;
        [[nodiscard]] const Field &GetField(unsigned index) const;

        [[nodiscard]] const ClassFunctionReference *GetFunction(
            const std::string &name,
            bool mutable_,
            const std::vector<Field> &parameters,
            bool vararg,
            const Field &result) const;

        [[nodiscard]] std::vector<ClassFunctionReference> GetConstructors() const;
        [[nodiscard]] std::optional<ClassFunctionReference> GetDestructor() const;

        void SetFields(Builder &builder, std::vector<ClassFieldReference> fields);
        void SetFunctions(std::vector<ClassFunctionReference> functions);

        [[nodiscard]] TypeId GetId() const override;
        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return c<length>_<name>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
        bool m_Opaque;
        std::vector<ClassFieldReference> m_Fields;
        std::vector<ClassFunctionReference> m_Functions;
    };

    class FunctionType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FunctionType>;

        explicit FunctionType(std::vector<Field> parameters, bool vararg, Field result);
        explicit FunctionType(std::vector<Field> parameters, bool vararg, Field result, Field self);

        [[nodiscard]] unsigned GetParameterCount() const;
        [[nodiscard]] const Field &GetParameter(unsigned index) const;
        [[nodiscard]] bool IsVarArg() const;
        [[nodiscard]] const Field &GetResult() const;
        [[nodiscard]] bool HasSelf() const;
        [[nodiscard]] const Field &GetSelf() const;

        [[nodiscard]] TypeId GetId() const override;
        llvm::PointerType *Gen(Builder &builder) const override;
        llvm::FunctionType *GenFunction(Builder &builder) const;

        /**
         * @return x<vararg?v><self?s><length>_<parameters...><result><self>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<Field> m_Parameters;
        bool m_VarArg;
        Field m_Result;
        Field m_Self;
    };
}

template<>
struct std::formatter<llove::TypePtr> : std::formatter<std::string_view>
{
    auto format(const llove::TypePtr &ptr, std::format_context &ctx) const
    {
        std::stringstream stream;
        ptr->Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};
