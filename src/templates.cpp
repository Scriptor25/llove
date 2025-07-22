#include <cli/templates.hpp>

bool YAML::convert<cli::OptionTemplate>::decode(const Node &node, cli::OptionTemplate &option)
{
    if (!node.IsMap() || !node["pattern"].IsDefined() || !node["type"].IsDefined())
        return false;

    option.Pattern = node["pattern"].as<std::set<std::string>>();
    option.Type = node["type"].as<cli::OptionTemplateType>();

    if (node["filter"].IsDefined())
        option.Filter = node["filter"].as<std::set<std::string>>();

    if (node["description"].IsDefined())
        option.Description = node["description"].as<std::string>();

    return true;
}

bool YAML::convert<cli::OptionTemplateType>::decode(const Node &node, cli::OptionTemplateType &type)
{
    static const std::map<std::string_view, cli::OptionTemplateType> map
    {
        { "flag", cli::OptionTemplateType_Flag },
        { "value", cli::OptionTemplateType_Value },
        { "array", cli::OptionTemplateType_Array },
    };

    const auto key = node.as<std::string>();
    if (!map.contains(key))
        return false;

    type = map.at(key);
    return true;
}
