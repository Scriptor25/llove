#pragma once

#include <map>
#include <string>
#include <vector>
#include <llove/class_template.hpp>
#include <llove/error.hpp>
#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/type.hpp>

namespace llove
{
    class Context
    {
    public:
        Context() = default;
        explicit Context(Context &parent);

        [[nodiscard]] TypePtr GetNamed(const std::string &id) const;
        void SetNamed(const std::string &id, TypePtr type);

        void Set(const std::string &hash, TypePtr type);

        template<typename T, typename... Args> requires std::is_base_of_v<Type, T>
        std::shared_ptr<T> GetOrCreate(Args &&... args)
        {
            auto type = std::make_shared<T>(std::forward<Args>(args)...);
            auto hash = type->Mangle();
            if (m_Types.contains(hash))
                return std::dynamic_pointer_cast<T>(m_Types.at(hash));

            auto parent = m_Parent;
            while (parent)
            {
                if (parent->m_Types.contains(hash))
                    return std::dynamic_pointer_cast<T>(parent->m_Types.at(hash));
                parent = parent->m_Parent;
            }

            m_Types.emplace(hash, type);
            return type;
        }

        VoidType::Ptr GetVoid();
        IntegerType::Ptr GetInteger(bool sign, unsigned bits);
        FloatType::Ptr GetFloat(unsigned bits);
        PointerType::Ptr GetPointer(bool mutable_);
        PointerType::Ptr GetPointer(TypePtr base, bool mutable_);
        ArrayType::Ptr GetArray(TypePtr base, unsigned size);
        StructType::Ptr GetStruct(std::vector<Parameter> fields);
        RangeType::Ptr GetRange(TypePtr entry);
        ClassType::Ptr GetClass(std::string name);
        FunctionType::Ptr GetFunction(
            std::vector<Field> parameters,
            bool vararg,
            Field result,
            std::optional<Field> self = std::nullopt);

        TypePtr TypeUnion(const TypePtr &left, const TypePtr &right);
        unsigned Difference(const TypePtr &left, const TypePtr &right);

        ClassTemplate &PushTemplate(
            std::string name,
            std::vector<std::pair<std::string, TemplateType::Ptr>> parameters);
        void PopTemplate();

        void EmplaceTemplate(std::string name, std::vector<std::pair<std::string, TemplateType::Ptr>> parameters);

        TypePtr InstantiateTemplateClass(std::string name, const std::vector<TypePtr> &arguments);
        void InstantiateReflections(Builder &builder);

        [[nodiscard]] TypePtr TemplateArgument(const std::string &name) const;

    private:
        Context *m_Parent = nullptr;

        std::map<std::string, TypePtr> m_Types;
        std::map<std::string, TypePtr> m_Named;

        std::vector<std::map<std::string, TemplateType::Ptr>> m_TemplateTypes;
        std::map<std::string, ClassTemplate> m_ClassTemplates;
        ClassTemplate *m_CurrentTemplate = nullptr;
        std::map<std::string, TypePtr> m_TemplateArguments;

        std::map<ClassType::Ptr, std::vector<ClassFunction>> m_Reflections;
    };

    template<typename T>
    T::Ptr As(TypePtr type)
    {
        Assert(type != nullptr, "type must not be null");
        auto ptr = std::dynamic_pointer_cast<T>(type);
        Assert(ptr != nullptr, "illegal cast from id {} to id {} (type {}) ", type->GetId(), T::ID, type);
        return ptr;
    }
}
