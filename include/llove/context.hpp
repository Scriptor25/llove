#pragma once

#include <map>
#include <string>
#include <vector>

#include <llove/field.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>

namespace llove
{
    class Context final
    {
    public:
        Context() = default;
        explicit Context(Context *parent);

        [[nodiscard]] Context *GetParent() const;

        [[nodiscard]] TypePtr GetNamed(const std::string &id) const;
        void SetNamed(const std::string &id, TypePtr type);

        void Set(std::string hash, TypePtr type);

        template<type_base T, typename... Args>
        std::shared_ptr<T> GetOrCreate(Args &&... args)
        {
            if (m_Parent)
                return m_Parent->GetOrCreate<T, Args...>(std::forward<Args>(args)...);

            auto type = std::make_unique<T>(std::forward<Args>(args)...);
            auto hash = type->Mangle();

            if (auto it = m_Types.find(hash); it != m_Types.end())
                return std::dynamic_pointer_cast<T>(it->second);

            auto &ref = m_Types[hash];
            ref = std::move(type);

            return std::dynamic_pointer_cast<T>(ref);
        }

        VoidType::Ptr GetVoid();
        VariadicType::Ptr GetVariadic();
        IntegerType::Ptr GetInteger(bool is_signed, unsigned bits);
        FloatType::Ptr GetFloat(unsigned bits);
        PointerType::Ptr GetPointer(bool is_mutable);
        PointerType::Ptr GetPointer(TypePtr base, bool is_mutable);
        ArrayType::Ptr GetArray(TypePtr base, unsigned size);
        StructType::Ptr GetStruct(std::vector<Parameter> fields);
        TupleType::Ptr GetTuple(std::vector<Field> fields);
        RangeType::Ptr GetRange(TypePtr entry);
        ClassType::Ptr GetClass(std::string name);
        FunctionType::Ptr GetFunction(
            Field result,
            std::vector<Field> parameters,
            bool variadic = false,
            std::optional<Field> self = std::nullopt);

        InstanceType::Ptr GetInstance(std::string name);
        InstanceType::Ptr GetInstance(std::string name, std::vector<TypePtr> arguments);

        IntegerType::Ptr GetBoolean();

        TypePtr TypeUnion(TypePtr left, TypePtr right);

        void PushTemplate(const std::vector<TemplateParameter> &parameters);
        void PopTemplate();

        void CreateTemplate(const std::string &name, TemplateInstancePtr instance);
        void CreateTemplate(const std::string &name, std::vector<TemplateParameter> parameters, GlobalPtr content);

        template<template_instance_base T>
        T &InstantiateTemplate(Builder &builder, const std::string &name, const std::vector<TypePtr> &type_arguments)
        {
            auto instance = InstantiateUniqueTemplate(&builder, name, type_arguments);
            return *dynamic_cast<T *>(instance);
        }

        template<template_instance_base T>
        T &InstantiateTemplate(const std::string &name, const std::vector<TypePtr> &type_arguments)
        {
            auto instance = InstantiateUniqueTemplate(nullptr, name, type_arguments);
            return *dynamic_cast<T *>(instance);
        }

        TemplateInstance *InstantiateUniqueTemplate(
            Builder *builder,
            const std::string &name,
            const std::vector<TypePtr> &type_arguments);

        [[nodiscard]] bool IsInstantiating() const;
        [[nodiscard]] TypePtr GetTemplateArgument(const std::string &name) const;

    private:
        Context *m_Parent = nullptr;

        std::map<std::string, TypePtr> m_Types;
        std::map<std::string, TypePtr> m_Named;

        std::vector<std::vector<TemplateParameter>> m_TemplateParameterStack;
        std::vector<std::map<std::string, TypePtr>> m_TemplateArgumentStack;
        std::map<std::string, Template> m_Templates;
        std::map<std::string, TemplateInstancePtr> m_Instances;
    };
}
