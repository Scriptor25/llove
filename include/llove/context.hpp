#pragma once

#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/function.hpp>
#include <llove/template.hpp>
#include <llove/type.hpp>
#include <map>
#include <string>
#include <vector>

namespace llove
{
    struct ClassReflection
    {
        std::map<std::string, TypePtr> Frame;

        ClassType::Ptr Class;
        std::vector<ClassFunction> Functions;
    };

    struct DefinitionReflection
    {
        std::map<std::string, TypePtr> Frame;

        Function Fun;
    };

    class Context
    {
    public:
        Context() = default;
        explicit Context(Context* parent);

        [[nodiscard]] Context* GetParent() const;

        [[nodiscard]] TypePtr GetNamed(const std::string& id) const;
        void SetNamed(
            const std::string& id,
            TypePtr type);

        void Set(
            std::string hash,
            TypePtr type);

        template<
            typename T,
            typename... Args>
        requires std::is_base_of_v<
            Type,
            T>
        std::shared_ptr<T> GetOrCreate(Args&&... args)
        {
            if (m_Parent)
                return m_Parent->GetOrCreate<T, Args...>(std::forward<Args>(args)...);

            auto type = std::make_unique<T>(std::forward<Args>(args)...);
            auto hash = type->Mangle();

            if (!m_Types.contains(hash))
                m_Types.emplace(hash, std::move(type));

            return std::dynamic_pointer_cast<T>(m_Types.at(hash));
        }

        VoidType::Ptr GetVoid();
        VariadicType::Ptr GetVariadic();
        IntegerType::Ptr GetInteger(
            bool is_signed,
            unsigned bits);
        FloatType::Ptr GetFloat(unsigned bits);
        PointerType::Ptr GetPointer(bool is_mutable);
        PointerType::Ptr GetPointer(
            TypePtr base,
            bool is_mutable);
        ArrayType::Ptr GetArray(
            TypePtr base,
            unsigned size);
        StructType::Ptr GetStruct(std::vector<Parameter> fields);
        RangeType::Ptr GetRange(TypePtr entry);
        ClassType::Ptr GetClass(std::string name);
        FunctionType::Ptr GetFunction(
            std::vector<Field> parameters,
            bool variadic,
            Field result,
            std::optional<Field> self = std::nullopt);

        IntegerType::Ptr GetBoolean();

        TypePtr TypeUnion(
            TypePtr left,
            TypePtr right);

        ClassTemplate& PushClassTemplate(
            bool is_export,
            std::string name,
            std::vector<std::pair<
                std::string,
                TemplateType::Ptr>> type_parameters,
            bool is_imported);
        void PopClassTemplate();
        ClassTemplate& EmplaceClassTemplate(
            bool is_export,
            std::string name,
            std::vector<std::pair<
                std::string,
                TemplateType::Ptr>> type_parameters);

        DefinitionTemplate& PushDefinitionTemplate(
            bool is_export,
            bool is_implicit,
            Location loc,
            std::string name,
            std::vector<std::pair<
                std::string,
                TemplateType::Ptr>> type_parameters,
            bool is_imported);
        void PopDefinitionTemplate();

        TypePtr InstantiateClass(
            std::string name,
            std::vector<TypePtr> type_arguments,
            bool is_imported);

        FunctionReference& InstantiateDefinition(
            Builder& builder,
            std::string name,
            std::vector<TypePtr> type_arguments,
            bool is_imported);

        void InstantiateReflections(Builder& builder);

        [[nodiscard]] TypePtr TemplateArgument(const std::string& name) const;

    private:
        Context* m_Parent = nullptr;

        std::map<std::string, TypePtr> m_Types;
        std::map<std::string, TypePtr> m_Named;

        std::vector<std::map<std::string, TemplateType::Ptr>> m_TemplateTypes;

        std::map<std::string, ClassTemplate> m_ClassTemplates;
        ClassTemplate* m_CurrentClassTemplate = nullptr;

        std::map<std::string, DefinitionTemplate> m_DefinitionTemplates;
        std::map<std::string, FunctionReference> m_DefinitionInstances;

        const std::map<std::string, TypePtr>* m_CurrentFrame = nullptr;

        std::vector<ClassReflection> m_ClassReflections;
        std::vector<DefinitionReflection> m_DefinitionReflections;
    };
}
