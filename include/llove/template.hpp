#pragma once

#include <llove/forward.hpp>
#include <llove/function.hpp>

namespace llove
{
    using TemplateParameter = std::pair<std::string, TemplateType::Ptr>;

    struct Template final
    {
        std::string Name;
        std::vector<TemplateParameter> Parameters;
        GlobalPtr Content;
        TemplateInstancePtr Default;
    };

    class TemplateInstance
    {
    public:
        explicit TemplateInstance() = default;
        virtual ~TemplateInstance() = default;
    };

    class FunctionTemplateInstance final : public TemplateInstance
    {
    public:
        explicit FunctionTemplateInstance(FunctionReference callee);

        const FunctionReference& GetCallee() const;

    private:
        FunctionReference m_Callee;
    };

    class TypeTemplateInstance final : public TemplateInstance
    {
    public:
        explicit TypeTemplateInstance(TypePtr type);

        TypePtr GetType() const;

    private:
        TypePtr m_Type;
    };
}
