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
        void Set(const std::string &id, const TypePtr &type);

        TypePtr GetVoid();
        TypePtr GetInt(bool sign, unsigned bits);
        TypePtr GetFlt(unsigned bits);
        TypePtr GetArray(const TypePtr &base, int64_t size);
        TypePtr GetStruct(const std::vector<Parameter> &parameters);

    private:
        TypePtr m_Void;
        std::map<bool, std::map<unsigned, TypePtr>> m_Int;
        std::map<unsigned, TypePtr> m_Flt;
        std::map<TypePtr, std::map<int64_t, TypePtr>> m_Array;
        std::map<ParameterHash, TypePtr> m_Struct;

        std::map<std::string, TypePtr> m_TypeMap;
    };
}
