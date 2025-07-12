#pragma once

#include <map>
#include <vector>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace llove
{
    class Type
    {
    public:
        virtual ~Type() = default;
        virtual llvm::Type *Gen(Builder &builder) const = 0;
        virtual std::string Mangle() const = 0;
        virtual std::ostream &Print(std::ostream &stream) const = 0;
    };

    class VoidType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VoidType>;

        explicit VoidType() = default;

        llvm::Type *Gen(Builder &builder) const override;

        /**
         * @return v
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;
    };

    class IntType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<IntType>;

        explicit IntType(bool sign, unsigned bits);

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

    class FltType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FltType>;

        explicit FltType(unsigned bits);

        llvm::Type *Gen(Builder &builder) const override;

        /**
         * @return f<bits>_
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        unsigned m_Bits;
    };

    class PtrType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<PtrType>;

        explicit PtrType(TypePtr base, bool mutable_);

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

        explicit StructType(std::vector<Parameter> fields);

        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return s<length>_<fields...>
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::vector<Parameter> m_Fields;
    };

    class ClassType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ClassType>;

        explicit ClassType(std::string name);
        explicit ClassType(std::string name, std::vector<Parameter> fields);

        llvm::StructType *Gen(Builder &builder) const override;

        /**
         * @return c<length>_<name>
         */
        std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

        void Set(std::vector<Parameter> fields);

    private:
        std::string m_Name;
        bool m_Opaque;
        std::vector<Parameter> m_Fields;
    };

    class FunctionType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FunctionType>;

        explicit FunctionType(std::vector<Field> parameters, bool vararg, Field result);
        explicit FunctionType(std::vector<Field> parameters, bool vararg, Field result, Field self);

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
