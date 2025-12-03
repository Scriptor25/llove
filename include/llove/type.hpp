#pragma once

#include <format>
#include <llove/class.hpp>
#include <llove/error.hpp>
#include <llove/forward.hpp>
#include <llove/parameter.hpp>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DerivedTypes.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace llove
{
    enum TypeId : unsigned
    {
        TypeId_Void = 'v',
        TypeId_Variadic = 'z',
        TypeId_Integer = 'i',
        TypeId_Float = 'f',
        TypeId_Pointer = 'p',
        TypeId_Array = 'a',
        TypeId_Struct = 's',
        TypeId_Tuple = 'm',
        TypeId_Range = 'r',
        TypeId_Class = 'c',
        TypeId_Function = 'x',
        TypeId_Template = 't',
    };

    class Type
    {
    public:
        using Ptr = std::shared_ptr<Type>;

        virtual ~Type() = default;

        virtual TypeId GetId() const = 0;

        virtual bool IsTemplate() const;
        virtual bool IsVoid() const;
        virtual bool IsVariadic() const;
        virtual bool IsInteger() const;
        virtual bool IsFloat() const;
        virtual bool IsPointer() const;
        virtual bool IsArray() const;
        virtual bool IsStruct() const;
        virtual bool IsTuple() const;
        virtual bool IsRange() const;
        virtual bool IsClass() const;
        virtual bool IsFunction() const;

        virtual unsigned SizeBits(Builder& builder);
        virtual llvm::Type* GenIR(Builder& builder) = 0;
        virtual llvm::DIType* GenDI(Builder& builder) = 0;
        virtual TypePtr Reflect(Context& context) const = 0;
        virtual bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const = 0;

        virtual std::string Mangle() const = 0;

        virtual std::ostream& Print(std::ostream& stream) const = 0;

        template<
            typename S,
            typename D>
        requires std::is_base_of_v<
                     Type,
                     S>
              && std::is_base_of_v<
                     Type,
                     D>
        static void Reflect(
            Context& context,
            const std::shared_ptr<S>& src,
            std::shared_ptr<D>& dst)
        {
            if (!src)
            {
                dst.reset();
                return;
            }

            auto cast = std::dynamic_pointer_cast<D>(src->Reflect(context));
            Assert(!!cast, "invalid reflection cast");
            dst = cast;
        }

    protected:
        llvm::Type* m_IRType = nullptr;
        llvm::DIType* m_DIType = nullptr;
    };

    class VoidType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VoidType>;
        static constexpr auto ID = TypeId_Void;

        explicit VoidType() = default;

        TypeId GetId() const override;
        bool IsVoid() const override;
        unsigned SizeBits(Builder& builder) override;
        llvm::Type* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return v
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;
    };

    class VariadicType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<VariadicType>;
        static constexpr auto ID = TypeId_Variadic;

        explicit VariadicType() = default;

        TypeId GetId() const override;
        bool IsVariadic() const override;
        llvm::Type* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return z
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;
    };

    class IntegerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<IntegerType>;
        static constexpr auto ID = TypeId_Integer;

        explicit IntegerType(
            bool is_signed,
            unsigned bits);

        bool IsSigned() const;
        unsigned GetBits() const;

        TypeId GetId() const override;
        bool IsInteger() const override;
        llvm::IntegerType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return <signed?i:u><bits>_
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

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

        unsigned GetBits() const;

        TypeId GetId() const override;
        bool IsFloat() const override;
        llvm::Type* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return f<bits>_
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        unsigned m_Bits;
    };

    class PointerType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<PointerType>;
        static constexpr auto ID = TypeId_Pointer;

        explicit PointerType(bool is_mutable);
        explicit PointerType(
            TypePtr base,
            bool is_mutable);

        TypePtr GetBase() const;
        bool IsMutable() const;
        bool IsOpaque() const;

        TypeId GetId() const override;
        bool IsPointer() const override;
        llvm::PointerType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return p<mutable?m:i><base>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        TypePtr m_Base;
        bool m_IsMutable;
    };

    class ArrayType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ArrayType>;
        static constexpr auto ID = TypeId_Array;

        explicit ArrayType(
            TypePtr base,
            unsigned count);

        TypePtr GetBase() const;
        unsigned GetCount() const;

        TypeId GetId() const override;
        bool IsArray() const override;
        llvm::ArrayType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return a<size>_<base>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

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

        bool HasField(const std::string& name) const;
        unsigned GetFieldIndex(const std::string& name) const;
        unsigned GetFieldCount() const;
        const Field& GetField(unsigned index) const;

        TypeId GetId() const override;
        bool IsStruct() const override;
        llvm::StructType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return s<length>_<fields...>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::vector<Parameter> m_Fields;
    };

    class TupleType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<TupleType>;
        static constexpr auto ID = TypeId_Tuple;

        explicit TupleType(std::vector<Field> fields);

        unsigned GetFieldCount() const;
        const Field& GetField(unsigned index) const;

        TypeId GetId() const override;
        bool IsTuple() const override;
        llvm::StructType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * m<length>_<fields...>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::vector<Field> m_Fields;
    };

    class RangeType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<RangeType>;
        static constexpr auto ID = TypeId_Range;

        explicit RangeType(TypePtr entry);

        TypePtr GetEntry() const;

        TypeId GetId() const override;
        bool IsRange() const override;
        llvm::StructType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return r<entry>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        TypePtr m_Entry;
    };

    class ClassType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<ClassType>;
        static constexpr auto ID = TypeId_Class;

        template<typename T>
        using Ref = std::pair<Ptr, T>;

        template<typename T>
        using OptRef = std::optional<std::pair<Ptr, T>>;

        template<typename T>
        using VecRef = std::vector<std::pair<Ptr, T>>;

        explicit ClassType(std::string name);

        const std::string& GetName() const;
        bool IsOpaque() const;

        bool InheritsFrom(const TypePtr& type) const;
        bool HasParentClass() const;
        Ptr GetParentClass() const;

        bool HasMember(const std::string& name) const;
        unsigned GetMemberIndex(const std::string& name) const;
        unsigned GetMemberCount() const;
        Field GetMember(unsigned index) const;

        void ForEachMember(
            const std::function<void(
                unsigned,
                const ClassMemberReference&)>& callback) const;

        OptRef<ClassFunctionReference> GetFunction(
            const Ptr& self,
            const std::string& name,
            bool is_mutable,
            const std::vector<Field>& parameters,
            bool has_variadic,
            const Field& result) const;

        bool HasFunction(const std::string& name) const;
        VecRef<ClassFunctionReference> GetFunctions(
            const Ptr& self,
            const std::string& name) const;
        VecRef<ClassFunctionReference> GetConstructors(const Ptr& self) const;
        OptRef<ClassFunctionReference> GetDestructor(const Ptr& self) const;

        void SetParentClass(Ptr parent_class_type);
        void SetMembers(std::vector<ClassMemberReference> members);
        void SetFunctions(std::vector<ClassFunctionReference> functions);

        TypeId GetId() const override;
        bool IsClass() const override;
        std::vector<llvm::Type*> GenIRElements(Builder& builder) const;
        std::pair<
            std::vector<llvm::Metadata*>,
            unsigned>
        GenDIElements(Builder& builder);
        llvm::StructType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return c<length>_<name>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Name;
        Ptr m_ParentClass;
        std::vector<ClassMemberReference> m_Members;
        std::vector<ClassFunctionReference> m_Functions;
    };

    class FunctionType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<FunctionType>;
        static constexpr auto ID = TypeId_Function;

        explicit FunctionType(
            Field result,
            std::vector<Field> parameters,
            bool variadic,
            std::optional<Field> self);

        unsigned GetParameterCount() const;
        const Field& GetParameter(unsigned index) const;
        bool HasVariadic() const;
        const Field& GetResult() const;
        const std::optional<Field>& GetSelf() const;

        TypeId GetId() const override;
        bool IsFunction() const override;
        llvm::PointerType* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        llvm::FunctionType* GenFunction(Builder& builder);
        llvm::DISubroutineType* GenDIFunction(Builder& builder);
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * @return x<variadic?v><self?s><length>_<parameters...><result><self>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    protected:
        llvm::FunctionType* m_IRFunction = nullptr;
        llvm::DISubroutineType* m_DIFunction = nullptr;

    private:
        Field m_Result;
        std::vector<Field> m_Parameters;
        bool m_Variadic;
        std::optional<Field> m_Self;
    };

    class TemplateType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<TemplateType>;
        static constexpr auto ID = TypeId_Template;

        explicit TemplateType(std::string name);

        TypeId GetId() const override;
        bool IsTemplate() const override;
        unsigned SizeBits(Builder& builder) override;
        llvm::Type* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * t<length>_<name>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Name;
    };

    class TemplateClassType final : public Type
    {
    public:
        using Ptr = std::shared_ptr<TemplateClassType>;
        static constexpr auto ID = TypeId_Template;

        explicit TemplateClassType(
            std::string name,
            std::vector<TypePtr> arguments);

        bool IsInstantiated() const;
        void Instantiate();

        TypeId GetId() const override;
        bool IsTemplate() const override;
        unsigned SizeBits(Builder& builder) override;
        llvm::Type* GenIR(Builder& builder) override;
        llvm::DIType* GenDI(Builder& builder) override;
        TypePtr Reflect(Context& context) const override;
        bool TypeInfo(
            Builder& builder,
            std::vector<llvm::Constant*>& dst) const override;

        /**
         * t<length>_<name><length>_<parameters...>
         */
        std::string Mangle() const override;

        std::ostream& Print(std::ostream& stream) const override;

    private:
        std::string m_Name;
        std::vector<TypePtr> m_Arguments;
        bool m_IsInstantiated = false;
    };

    template<typename T>
    T::Ptr As(TypePtr type)
    {
        Assert(!!type, "type must not be null");
        auto ptr = std::dynamic_pointer_cast<T>(type);
        Assert(!!ptr, "illegal cast from {} to {} (type {}) ", type->GetId(), T::ID, type);
        return ptr;
    }

    unsigned Difference(
        const TypePtr& left,
        const TypePtr& right);
}

template<typename T>
requires std::is_base_of_v<llove::Type, T>
struct std::formatter<std::shared_ptr<T>> : std::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(
        const std::shared_ptr<T>& ptr,
        FormatContext& ctx) const
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
    auto format(
        const llove::TypeId& id,
        FormatContext& ctx) const
    {
        static const std::map<llove::TypeId, const char*> map{
            {     llove::TypeId_Void,     "void" },
            { llove::TypeId_Variadic, "variadic" },
            {  llove::TypeId_Integer,  "integer" },
            {    llove::TypeId_Float,    "float" },
            {  llove::TypeId_Pointer,  "pointer" },
            {    llove::TypeId_Array,    "array" },
            {   llove::TypeId_Struct,   "struct" },
            {    llove::TypeId_Tuple,    "tuple" },
            {    llove::TypeId_Range,    "range" },
            {    llove::TypeId_Class,    "class" },
            { llove::TypeId_Function, "function" },
            { llove::TypeId_Template, "template" },
        };
        return std::formatter<std::string_view>::format(map.at(id), ctx);
    }
};
