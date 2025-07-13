#pragma once

#include <llove/forward.hpp>

namespace llove
{
    class Value
    {
    public:
        static ValuePtr CreateR(TypePtr type, llvm::Value *value);
        static ValuePtr CreateL(TypePtr type, llvm::Value *pointer, bool mutable_);

        virtual ~Value() = default;

        TypePtr GetType() const;

        virtual bool IsReferenceable() const = 0;
        virtual bool IsMutable() const = 0;
        virtual llvm::Value *Load(Builder &builder) const = 0;
        virtual void Store(Builder &builder, llvm::Value *value, bool volatile_ = false) const = 0;
        virtual ValuePtr Reference(Builder &builder) const = 0;
        virtual llvm::Value *GetPointer() const = 0;

        Field AsField() const;

    protected:
        explicit Value(TypePtr type);

        TypePtr m_Type;
    };

    class RValue final : public Value
    {
    public:
        explicit RValue(TypePtr type, llvm::Value *value);

        bool IsReferenceable() const override;
        bool IsMutable() const override;
        llvm::Value *Load(Builder &builder) const override;
        void Store(Builder &builder, llvm::Value *value, bool volatile_) const override;
        ValuePtr Reference(Builder &builder) const override;
        llvm::Value *GetPointer() const override;

    private:
        llvm::Value *m_Value;
    };

    class LValue final : public Value
    {
    public:
        explicit LValue(TypePtr type, llvm::Value *pointer, bool mutable_);

        bool IsReferenceable() const override;
        bool IsMutable() const override;
        llvm::Value *Load(Builder &builder) const override;
        void Store(Builder &builder, llvm::Value *value, bool volatile_) const override;
        ValuePtr Reference(Builder &builder) const override;
        llvm::Value *GetPointer() const override;

    private:
        llvm::Value *m_Pointer;
        bool m_Mutable;
    };
}
