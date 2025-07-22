#pragma once

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

    struct OptionTemplate final
    {
        std::set<std::string> Pattern;
        OptionTemplateType Type = OptionTemplateType_Flag;
        std::set<std::string> Filter;
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
