#pragma once

#include <map>
#include <string>
#include <vector>
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
        ArrayType::Ptr GetArray(TypePtr base, unsigned size);
        StructType::Ptr GetStruct(std::vector<Parameter> fields);
        RangeType::Ptr GetRange(TypePtr entry);
        ClassType::Ptr GetClass(std::string name);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result, Field self);

        [[nodiscard]] bool HasClass(const std::string &name) const;

        TypePtr DetermineHigherOrder(const TypePtr &left, const TypePtr &right);

    private:
        VoidType::Ptr m_Void;
        std::map<bool, std::map<unsigned, IntegerType::Ptr>> m_Integer;
        std::map<unsigned, FloatType::Ptr> m_Float;
        std::map<TypePtr, std::map<bool, PointerType::Ptr>> m_Pointer;
        std::map<TypePtr, std::map<unsigned, ArrayType::Ptr>> m_Array;
        std::map<std::string, StructType::Ptr> m_Struct;
        std::map<TypePtr, RangeType::Ptr> m_Range;
        std::map<std::string, ClassType::Ptr> m_Class;
        std::map<std::string, std::map<bool, std::map<std::string, std::map<std::string, FunctionType::Ptr>>>>
        m_Function;

        std::map<std::string, TypePtr> m_TypeMap;
    };

    template<typename T>
    typename T::Ptr As(TypePtr type)
    {
        auto ptr = std::dynamic_pointer_cast<T>(type);
        Assert(ptr != nullptr, "illegal cast from id {} to id {} (type {}) ", type->GetId(), T::ID, type);
        return ptr;
    }
}
