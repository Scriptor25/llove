#pragma once

#include <memory>
#include <set>
#include <string>
#include <yaml-cpp/yaml.h>

namespace cli
{
    enum OptionTemplateType
    {
        OptionTemplateType_Flag,
        OptionTemplateType_Value,
        OptionTemplateType_Array,
    };

    enum FilterTemplateType
    {
        FilterTemplateType_Integer,
        FilterTemplateType_String,
        FilterTemplateType_Value,
    };

    struct FilterTemplate
    {
        virtual ~FilterTemplate() = default;

        virtual void Validate(const std::string &pat, const std::string &val) const;
        virtual void Stringify(std::string &filter_str) const;

        FilterTemplateType Type;
    };

    struct FilterTemplateValue final : FilterTemplate
    {
        void Validate(const std::string &pat, const std::string &val) const override;
        void Stringify(std::string &filter_str) const override;

        std::set<std::string> Values;
    };

    struct OptionTemplate final
    {
        std::set<std::string> Pattern;
        OptionTemplateType Type = OptionTemplateType_Flag;
        std::unique_ptr<FilterTemplate> Filter;
        std::string Description;
    };
}

namespace YAML
{
    template<>
    struct convert<cli::OptionTemplate> final
    {
        static bool decode(const Node &node, cli::OptionTemplate &option);
    };

    template<>
    struct convert<cli::OptionTemplateType> final
    {
        static bool decode(const Node &node, cli::OptionTemplateType &type);
    };

    template<>
    struct convert<std::unique_ptr<cli::FilterTemplate>> final
    {
        static bool decode(const Node &node, std::unique_ptr<cli::FilterTemplate> &ptr);
    };

    template<typename T>
    struct convert<std::set<T>> final
    {
        static bool decode(const Node &node, std::set<T> &set)
        {
            if (node.IsSequence())
            {
                for (auto &value : node)
                    set.insert(value.as<T>());
            }
            else
            {
                set.insert(node.as<T>());
            }
            return true;
        }
    };
}
