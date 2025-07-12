#pragma once

#include <map>
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
        IntType::Ptr GetInt(bool sign, unsigned bits);
        FltType::Ptr GetFlt(unsigned bits);
        PtrType::Ptr GetPtr(TypePtr base, bool mutable_);
        ArrayType::Ptr GetArray(TypePtr base, int64_t size);
        StructType::Ptr GetStruct(std::vector<Parameter> parameters);
        ClassType::Ptr GetClass(const std::string &name);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result);
        FunctionType::Ptr GetFunction(std::vector<Field> parameters, bool vararg, Field result, Field self);

    private:
        VoidType::Ptr m_Void;
        std::map<bool, std::map<unsigned, IntType::Ptr>> m_Int;
        std::map<unsigned, FltType::Ptr> m_Flt;
        std::map<TypePtr, std::map<bool, PtrType::Ptr>> m_Ptr;
        std::map<TypePtr, std::map<int64_t, ArrayType::Ptr>> m_Array;
        std::map<std::string, StructType::Ptr> m_Struct;
        std::map<std::string, ClassType::Ptr> m_Class;
        std::map<std::string, std::map<bool, std::map<std::string, std::map<std::string, FunctionType::Ptr>>>>
        m_Function;

        std::map<std::string, TypePtr> m_TypeMap;
    };
}
