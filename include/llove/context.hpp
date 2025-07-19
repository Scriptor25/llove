#pragma once

#include <map>
#include <string>
#include <vector>
#include <llove/class.hpp>
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

        VoidType::Ptr GetVoid();
        IntegerType::Ptr GetInteger(bool sign, unsigned bits);
        FloatType::Ptr GetFloat(unsigned bits);
        PointerType::Ptr GetPointer(bool mutable_);
        PointerType::Ptr GetPointer(TypePtr base, bool mutable_);
        ArrayType::Ptr GetArray(TypePtr base, int64_t size);
        StructType::Ptr GetStruct(std::vector<ClassFieldReference> fields);
        ClassType::Ptr GetClass(std::string name);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result, Field self);

        bool HasClass(const std::string &name) const;

        TypePtr DetermineHigherOrder(TypePtr left, TypePtr right);

    private:
        VoidType::Ptr m_Void;
        std::map<bool, std::map<unsigned, IntegerType::Ptr>> m_Integer;
        std::map<unsigned, FloatType::Ptr> m_Float;
        std::map<TypePtr, std::map<bool, PointerType::Ptr>> m_Pointer;
        std::map<TypePtr, std::map<int64_t, ArrayType::Ptr>> m_Array;
        std::map<std::string, StructType::Ptr> m_Struct;
        std::map<std::string, ClassType::Ptr> m_Class;
        std::map<std::string, std::map<bool, std::map<std::string, std::map<std::string, FunctionType::Ptr>>>>
        m_Function;

        std::map<std::string, TypePtr> m_TypeMap;
    };

    template<typename T>
    typename T::Ptr As(TypePtr type)
    {
        return std::dynamic_pointer_cast<T>(std::move(type));
    }
}
