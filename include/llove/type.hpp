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
        TypeId_Class,
        TypeId_Function,
    };

    class Type
    {
    public:
        virtual ~Type() = default;
        virtual TypeId GetId() const = 0;
        virtual llvm::Type *Gen(Builder &builder) const = 0;
        virtual std::string Mangle() const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    class VoidType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VoidType>;

        explicit VoidType() = default;

        TypeId GetId() const override;
        llvm::Type *Gen(Builder &builder) const override;

        /**
         * @return v
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;
    };

    class IntegerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<IntegerType>;

        explicit IntegerType(bool sign, unsigned bits);

        bool IsSigned() const;
        unsigned GetBits() const;

        TypeId GetId() const override;
        llvm::IntegerType *Gen(Builder &builder) const override;

        /**
         * @return <sign?i:u><bits>_
         */
        std::string Mangle() const override;

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

        unsigned GetBits() const;

        TypeId GetId() const override;
        llvm::Type *Gen(Builder &builder) const override;

        /**
         * @return f<bits>_
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        unsigned m_Bits;
    };

    class PointerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<PointerType>;

        explicit PointerType(TypePtr base, bool mutable_);

        TypePtr GetBase() const;
        bool IsMutable() const;
        bool IsOpaque() const;

        TypeId GetId() const override;
        llvm::PointerType *Gen(Builder &builder) const override;

        /**
         * @return p<mutable?m:i><base>
         */
        std::string Mangle() const override;

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

        TypePtr GetBase() const;
        unsigned GetSize() const;

        TypeId GetId() const override;
        llvm::ArrayType *Gen(Builder &builder) const override;

        /**
         * @return a<size>_<base>
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Base;
        unsigned m_Size;
    };

    class StructType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<StructType>;

        explicit StructType(std::vector<ClassField> fields);

        unsigned GetFieldIndex(const std::string &name) const;

        unsigned GetFieldCount() const;
        const Field &GetField(unsigned index) const;

        TypeId GetId() const override;
        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return s<length>_<fields...>
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<ClassField> m_Fields;
    };

    class ClassType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ClassType>;

        explicit ClassType(std::string name);
        explicit ClassType(std::string name, std::vector<ClassField> fields, std::vector<ClassFunctionInfo> functions);

        const std::string &GetName() const;
        bool IsOpaque() const;

        unsigned GetFieldIndex(const std::string &name) const;

        unsigned GetFieldCount() const;
        const Field &GetField(unsigned index) const;

        const ClassFunctionInfo *GetFunction(
            const std::string &name,
            bool mutable_,
            const std::vector<Field> &parameters,
            bool vararg,
            const Field &result) const;

        std::vector<const ClassFunctionInfo *> GetCreates() const;

        void SetFields(Builder &builder, std::vector<ClassField> fields);
        void SetFunctions(std::vector<ClassFunctionInfo> functions);

        TypeId GetId() const override;
        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return c<length>_<name>
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
        bool m_Opaque;
        std::vector<ClassField> m_Fields;
        std::vector<ClassFunctionInfo> m_Functions;
    };

    class FunctionType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FunctionType>;

        explicit FunctionType(std::vector<Field> parameters, bool vararg, Field result);
        explicit FunctionType(std::vector<Field> parameters, bool vararg, Field result, Field self);

        unsigned GetParameterCount() const;
        const Field &GetParameter(unsigned index) const;
        bool IsVarArg() const;
        const Field &GetResult() const;
        bool HasSelf() const;
        const Field &GetSelf() const;

        TypeId GetId() const override;
        llvm::FunctionType *Gen(Builder &builder) const override;

        /**
         * @return x<vararg?v><self?s><length>_<parameters...><result><self>
         */
        std::string Mangle() const override;

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
