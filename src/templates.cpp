#include <functional>

#include <cli/templates.hpp>

#include <llove/error.hpp>

using namespace std::placeholders;

bool YAML::convert<cli::OptionTemplate>::decode(const Node &node, cli::OptionTemplate &option)
{
    if (!node.IsMap() || !node["pattern"].IsDefined() || !node["type"].IsDefined())
    {
        return false;
    }

    option.Pattern = node["pattern"].as<std::set<std::string>>();
    option.Type = node["type"].as<cli::OptionTemplateType>();

    if (option.Type != cli::OptionTemplateType_Flag)
    {
        if (!node["filter"].IsDefined())
        {
            return false;
        }
        option.Filter = node["filter"].as<std::unique_ptr<cli::FilterTemplate>>();
    }

    if (node["description"].IsDefined())
    {
        option.Description = node["description"].as<std::string>();
    }

    return true;
}

bool YAML::convert<cli::OptionTemplateType>::decode(const Node &node, cli::OptionTemplateType &type)
{
    static const std::map<std::string_view, cli::OptionTemplateType> map{
        { "flag", cli::OptionTemplateType_Flag },
        { "value", cli::OptionTemplateType_Value },
        { "array", cli::OptionTemplateType_Array },
    };

    const auto key = node.as<std::string>();
    if (!map.contains(key))
    {
        return false;
    }

    type = map.at(key);
    return true;
}

bool YAML::convert<std::unique_ptr<cli::FilterTemplate>>::decode(
    const Node &node,
    std::unique_ptr<cli::FilterTemplate> &ptr)
{
    if (node.IsSequence())
    {
        auto value_ptr = std::make_unique<cli::FilterTemplateValue>();
        value_ptr->Type = cli::FilterTemplateType_Value;
        value_ptr->Values = node.as<std::set<std::string>>();

        ptr = std::move(value_ptr);
        return true;
    }

    const auto value = node.as<std::string>();

    if (value == "integer")
    {
        ptr = std::make_unique<cli::FilterTemplate>();
        ptr->Type = cli::FilterTemplateType_Integer;
        return true;
    }

    if (value == "string")
    {
        ptr = std::make_unique<cli::FilterTemplate>();
        ptr->Type = cli::FilterTemplateType_String;
        return true;
    }

    return false;
}

void cli::FilterTemplate::Validate(const std::string &pat, const std::string &val) const
{
    auto is_digit = [](const char c)
    {
        return std::isdigit(c);
    };

    switch (Type)
    {
    case FilterTemplateType_Integer:
        llove::Assert(
            std::ranges::all_of(val, is_digit),
            "illegal use of argument '{}': value '{}' does not match filter "
            "'integer'",
            pat,
            val);
        break;
    case FilterTemplateType_String:
        break;
    default:
        llove::Error("undefined filter type");
    }
}

void cli::FilterTemplate::Stringify(std::string &filter_str) const
{
    switch (Type)
    {
    case FilterTemplateType_Integer:
        filter_str = "integer";
        break;
    case FilterTemplateType_String:
        filter_str = "string";
        break;
    default:
        filter_str = "undefined";
        break;
    }
}

void cli::FilterTemplateValue::Validate(const std::string &pat, const std::string &val) const
{
    llove::Assert(
        Values.empty() || Values.contains(val),
        "illegal use of argument '{}': value '{}' does not match filter '[...]'",
        pat,
        val);
}

void cli::FilterTemplateValue::Stringify(std::string &filter_str) const
{
    filter_str += '[';
    for (auto value = Values.begin(); value != Values.end(); ++value)
    {
        if (value != Values.begin())
        {
            filter_str += '|';
        }
        filter_str += '"' + *value + '"';
    }
    filter_str += ']';
}
