#pragma once

#include <format>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <llove/class.hpp>
#include <llove/error.hpp>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>

namespace llove
{
    enum TypeId
    {
        TypeId_Template,
        TypeId_Void,
        TypeId_Variadic,
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
        using Ptr = std::shared_ptr<Type>;

        virtual ~Type() = default;

        [[nodiscard]] virtual TypeId GetId() const = 0;

        [[nodiscard]] virtual bool IsTemplate() const;
        [[nodiscard]] virtual bool IsVoid() const;
        [[nodiscard]] virtual bool IsVariadic() const;
        [[nodiscard]] virtual bool IsArgPointer() const;
        [[nodiscard]] virtual bool IsInteger() const;
        [[nodiscard]] virtual bool IsFloat() const;
        [[nodiscard]] virtual bool IsPointer() const;
        [[nodiscard]] virtual bool IsArray() const;
        [[nodiscard]] virtual bool IsStruct() const;
        [[nodiscard]] virtual bool IsRange() const;
        [[nodiscard]] virtual bool IsClass() const;
        [[nodiscard]] virtual bool IsFunction() const;

        virtual unsigned SizeBits(Builder &builder);
        virtual llvm::Type *GenIR(Builder &builder) = 0;
        virtual llvm::DIType *GenDI(Builder &builder) = 0;
        virtual TypePtr Reflect(Context &context) const = 0;

        [[nodiscard]] virtual std::string Mangle() const = 0;

        virtual std::ostream &Print(std::ostream &stream) const = 0;

        template<typename S, typename D> requires std::is_base_of_v<Type, S> && std::is_base_of_v<Type, D>
        static void Reflect(Context &context, std::shared_ptr<S> src, std::shared_ptr<D> &dst)
        {
            if (!src)
            {
                dst = nullptr;
                return;
            }

            auto cast = std::dynamic_pointer_cast<D>(src->Reflect(context));
            Assert(cast != nullptr, "invalid reflection cast");
            dst = cast;
        }

    protected:
        llvm::Type *m_IRType = nullptr;
        llvm::DIType *m_DIType = nullptr;
    };

    class TemplateType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<TemplateType>;
        static constexpr auto ID = TypeId_Template;

        explicit TemplateType(std::string name);

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsTemplate() const override;
        unsigned SizeBits(Builder &builder) override;
        llvm::Type *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
    };

    class ClassTemplateType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ClassTemplateType>;
        static constexpr auto ID = TypeId_Template;

        explicit ClassTemplateType(std::string name, std::vector<TypePtr> arguments);

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsTemplate() const override;
        unsigned SizeBits(Builder &builder) override;
        llvm::Type *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        std::string m_Name;
        std::vector<TypePtr> m_Arguments;
    };

    class VoidType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VoidType>;
        static constexpr auto ID = TypeId_Void;

        explicit VoidType() = default;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsVoid() const override;
        unsigned SizeBits(Builder &builder) override;
        llvm::Type *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

        /**
         * @return v
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;
    };

    class VariadicType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VariadicType>;
        static constexpr auto ID = TypeId_Variadic;

        explicit VariadicType() = default;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsVariadic() const override;
        llvm::Type *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

        /**
         * @return z
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;
    };

    class IntegerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<IntegerType>;
        static constexpr auto ID = TypeId_Integer;

        explicit IntegerType(bool is_signed, unsigned bits);

        [[nodiscard]] bool IsSigned() const;
        [[nodiscard]] unsigned GetBits() const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsInteger() const override;
        llvm::IntegerType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

        /**
         * @return <signed?i:u><bits>_
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        bool m_IsSigned;
        unsigned m_Bits;
    };

    class FloatType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FloatType>;
        static constexpr auto ID = TypeId_Float;

        explicit FloatType(unsigned bits);

        [[nodiscard]] unsigned GetBits() const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsFloat() const override;
        llvm::Type *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

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
        static constexpr auto ID = TypeId_Pointer;

        explicit PointerType(TypePtr base, bool mutable_);

        [[nodiscard]] TypePtr GetBase() const;
        [[nodiscard]] bool IsMutable() const;
        [[nodiscard]] bool IsOpaque() const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsPointer() const override;
        llvm::PointerType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

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
        static constexpr auto ID = TypeId_Array;

        explicit ArrayType(TypePtr base, unsigned count);

        [[nodiscard]] TypePtr GetBase() const;
        [[nodiscard]] unsigned GetCount() const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsArray() const override;
        llvm::ArrayType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

        /**
         * @return a<size>_<base>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    private:
        TypePtr m_Base;
        unsigned m_Count;
    };

    class StructType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<StructType>;
        static constexpr auto ID = TypeId_Struct;

        explicit StructType(std::vector<Parameter> fields);

        [[nodiscard]] bool HasField(const std::string &name) const;
        [[nodiscard]] unsigned GetFieldIndex(const std::string &name) const;
        [[nodiscard]] unsigned GetFieldCount() const;
        [[nodiscard]] const Field &GetField(unsigned index) const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsStruct() const override;
        llvm::StructType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

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
        static constexpr auto ID = TypeId_Range;

        explicit RangeType(TypePtr entry);

        [[nodiscard]] TypePtr GetEntry() const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsRange() const override;
        llvm::StructType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

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
        static constexpr auto ID = TypeId_Class;

        explicit ClassType(std::string name);
        explicit ClassType(
            std::string name,
            std::vector<ClassFieldReference> fields,
            std::vector<ClassFunctionReference> functions);

        [[nodiscard]] const std::string &GetName() const;
        [[nodiscard]] bool IsOpaque() const;

        [[nodiscard]] bool HasField(const std::string &name) const;
        [[nodiscard]] unsigned GetFieldIndex(const std::string &name) const;
        [[nodiscard]] unsigned GetFieldCount() const;
        [[nodiscard]] const Field &GetField(unsigned index) const;

        [[nodiscard]] std::optional<ClassFunctionReference> GetFunction(
            const std::string &name,
            bool mutable_,
            const std::vector<Field> &parameters,
            bool variadic,
            const Field &result) const;

        [[nodiscard]] bool HasFunction(const std::string &name) const;
        [[nodiscard]] std::vector<ClassFunctionReference> GetFunctions(const std::string &name) const;
        [[nodiscard]] std::vector<ClassFunctionReference> GetConstructors() const;
        [[nodiscard]] std::optional<ClassFunctionReference> GetDestructor() const;

        void SetFields(std::vector<ClassFieldReference> fields);
        void SetFunctions(std::vector<ClassFunctionReference> functions);

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsClass() const override;
        llvm::StructType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        TypePtr Reflect(Context &context) const override;

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
        static constexpr auto ID = TypeId_Function;

        explicit FunctionType(std::vector<Field> parameters, bool variadic, Field result, std::optional<Field> self);

        [[nodiscard]] unsigned GetParameterCount() const;
        [[nodiscard]] const Field &GetParameter(unsigned index) const;
        [[nodiscard]] bool HasVariadic() const;
        [[nodiscard]] const Field &GetResult() const;
        [[nodiscard]] const std::optional<Field> &GetSelf() const;

        [[nodiscard]] TypeId GetId() const override;
        [[nodiscard]] bool IsFunction() const override;
        llvm::PointerType *GenIR(Builder &builder) override;
        llvm::DIType *GenDI(Builder &builder) override;
        llvm::FunctionType *GenFunction(Builder &builder);
        llvm::DISubroutineType *GenDIFunction(Builder &builder);
        TypePtr Reflect(Context &context) const override;

        /**
         * @return x<variadic?v><self?s><length>_<parameters...><result><self>
         */
        [[nodiscard]] std::string Mangle() const override;

        std::ostream &Print(std::ostream &stream) const override;

    protected:
        llvm::FunctionType *m_IRFunction = nullptr;
        llvm::DISubroutineType *m_DIFunction = nullptr;

    private:
        std::vector<Field> m_Parameters;
        bool m_Variadic;
        Field m_Result;
        std::optional<Field> m_Self;
    };

    template<typename T>
    T::Ptr As(TypePtr type)
    {
        Assert(type != nullptr, "type must not be null");
        auto ptr = std::dynamic_pointer_cast<T>(type);
        Assert(ptr != nullptr, "illegal cast from {} to {} (type {}) ", type->GetId(), T::ID, type);
        return ptr;
    }
}

template<typename T> requires std::is_base_of_v<llove::Type, T>
struct std::formatter<std::shared_ptr<T>> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const std::shared_ptr<T> &ptr, FormatContext &ctx) const
    {
        std::stringstream stream;
        ptr->Print(stream);
        return std::formatter<std::string_view>::format(stream.view(), ctx);
    }
};

template<>
struct std::formatter<llove::TypeId> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const llove::TypeId &id, FormatContext &ctx) const
    {
        static const std::map<llove::TypeId, const char *> map
        {
            { llove::TypeId_Void, "void" },
            { llove::TypeId_Integer, "integer" },
            { llove::TypeId_Float, "float" },
            { llove::TypeId_Pointer, "pointer" },
            { llove::TypeId_Array, "array" },
            { llove::TypeId_Struct, "struct" },
            { llove::TypeId_Range, "range" },
            { llove::TypeId_Class, "class" },
            { llove::TypeId_Function, "function" },
        };
        return std::formatter<std::string_view>::format(map.at(id), ctx);
    }
};
