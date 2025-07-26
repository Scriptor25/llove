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

        [[nodiscard]] TypePtr Get(const std::string &id) const;
        void Set(const std::string &id, TypePtr type);

        template<typename T, typename... Args> requires std::is_base_of_v<Type, T>
        std::shared_ptr<T> GetOrCreate(Args &&... args)
        {
            auto type = std::make_shared<T>(std::forward<Args>(args)...);
            auto hash = type->Mangle();
            if (m_Types.contains(hash))
                return std::dynamic_pointer_cast<T>(m_Types.at(hash));
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
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result, Field self);

        TypePtr GetMax(const TypePtr &left, const TypePtr &right);

        ClassTemplate &PushTemplate(
            std::string name,
            std::vector<std::pair<std::string, TemplateType::Ptr>> parameters);
        void PopTemplate();

        void EmplaceTemplate(std::string name, std::vector<std::pair<std::string, TemplateType::Ptr>> parameters);

        ClassType::Ptr InstantiateTemplateClass(
            Builder &builder,
            std::string name,
            const std::vector<TypePtr> &arguments);

    private:
        std::map<std::string, TypePtr> m_Types;
        std::map<std::string, TypePtr> m_Named;

        std::vector<std::map<std::string, TemplateType::Ptr>> m_TemplateTypes;

        std::map<std::string, ClassTemplate> m_ClassTemplates;
    };

    template<typename T>
    typename T::Ptr As(TypePtr type)
    {
        Assert(type != nullptr, "type must not be null");
        auto ptr = std::dynamic_pointer_cast<T>(type);
        Assert(ptr != nullptr, "illegal cast from id {} to id {} (type {}) ", type->GetId(), T::ID, type);
        return ptr;
    }
}
