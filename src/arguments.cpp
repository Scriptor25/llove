#include <config.h>

#include <ranges>

#include <cli/arguments.hpp>
#include <llove/error.hpp>

static std::string config_yaml = { reinterpret_cast<const char *>(config_data), config_data_len };

static std::vector<std::string> split_string(
    std::string value,
    const char delimiter)
{
    std::vector<std::string> values;
    for (size_t pos; (pos = value.find(delimiter)) != std::string::npos;)
    {
        values.push_back(value.substr(0, pos));
        value = value.substr(pos + 1);
    }
    if (!value.empty())
    {
        values.push_back(value);
    }
    return values;
}

cli::Arguments::Arguments(
    const char *const*begin,
    const char *const*end)
{
    m_Templates = YAML::Load(config_yaml).as<std::map<std::string, OptionTemplate>>();

    for (auto i = begin; i != end; ++i)
    {
        auto arg = split_string(*i, '=');
        auto &pat = arg.at(0);

        auto match = false;
        for (auto &[id, template_] : m_Templates)
        {
            if (!template_.Pattern.contains(pat))
            {
                continue;
            }

            match = true;
            switch (template_.Type)
            {
            case OptionTemplateType_Flag:
                llove::Assert(arg.size() == 1, "illegal use of argument '{}': must be a flag", pat);
                m_Flags.insert(id);
                break;
            case OptionTemplateType_Value:
            {
                std::string val;
                if (arg.size() == 2)
                {
                    val = arg.at(1);
                }
                else if (arg.size() == 1)
                {
                    val = *++i;
                }
                else
                {
                    llove::Error("illegal use of argument '{}': must be a value", pat);
                }

                template_.Filter->Validate(pat, val);
                m_Values.emplace(id, val);
                break;
            }
            case OptionTemplateType_Array:
            {
                std::string val_str;
                if (arg.size() == 2)
                {
                    val_str = arg.at(1);
                }
                else if (arg.size() == 1)
                {
                    val_str = *++i;
                }
                else
                {
                    llove::Error("illegal use of argument '{}': must be an array", pat);
                }

                auto vals = split_string(val_str, ',');
                for (auto &val : vals)
                {
                    template_.Filter->Validate(pat, val);
                }
                if (m_Arrays.contains(id))
                {
                    m_Arrays.at(id).insert(m_Arrays.at(id).end(), vals.begin(), vals.end());
                }
                else
                {
                    m_Arrays.emplace(id, std::move(vals));
                }
                break;
            }
            }
        }

        if (match)
            continue;

        llove::Assert(m_Filename.empty(), "illegal positional argument '{}': only one filename can be specified", *i);
        m_Filename = *i;
    }
}

std::string cli::Arguments::BuildCommandLine() const
{
    std::stringstream stream;

    auto first = true;
    for (auto &id : m_Flags)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ' ';
        }
        stream << *m_Templates.at(id).Pattern.begin();
    }
    for (auto &[id, value] : m_Values)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ' ';
        }
        stream << *m_Templates.at(id).Pattern.begin() << '=' << value;
    }
    for (auto &[id, values] : m_Arrays)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ' ';
        }
        stream << *m_Templates.at(id).Pattern.begin() << '=';
        for (auto i = values.begin(); i != values.end(); ++i)
        {
            if (i != values.begin())
            {
                stream << ',';
            }
            stream << *i;
        }
    }

    return stream.str();
}

const std::map<std::string, cli::OptionTemplate> &cli::Arguments::templates() const
{
    return m_Templates;
}

const std::string &cli::Arguments::filename() const
{
    return m_Filename;
}

bool cli::Arguments::has_none_except(const std::set<std::string> &id_set) const
{
    const auto pred = [&](const std::string &id) -> bool
    {
        return id_set.contains(id);
    };

    return m_Filename.empty()
           && std::ranges::all_of(m_Flags, pred)
           && std::ranges::all_of(m_Values | std::views::keys, pred)
           && std::ranges::all_of(m_Arrays | std::views::keys, pred);
}

bool cli::Arguments::flag(const std::string &id) const
{
    return m_Flags.contains(id);
}

std::optional<std::string> cli::Arguments::value(const std::string &id) const
{
    return m_Values.contains(id) ? std::optional{ m_Values.at(id) } : std::nullopt;
}

std::optional<std::vector<std::string>> cli::Arguments::array(const std::string &id) const
{
    return m_Arrays.contains(id) ? std::optional{ m_Arrays.at(id) } : std::nullopt;
}

bool cli::Arguments::value(
    const std::string &id,
    std::string &dst) const
{
    if (!m_Values.contains(id))
    {
        return false;
    }
    dst = m_Values.at(id);
    return true;
}

bool cli::Arguments::array(
    const std::string &id,
    std::vector<std::string> &dst) const
{
    if (!m_Arrays.contains(id))
    {
        return false;
    }
    dst = m_Arrays.at(id);
    return true;
}

bool cli::Arguments::set(
    const std::string &id,
    std::set<std::string> &dst) const
{
    if (!m_Arrays.contains(id))
    {
        return false;
    }
    auto &array = m_Arrays.at(id);
    dst = std::set(array.begin(), array.end());
    return true;
}

bool cli::Arguments::has_value(const std::string &id) const
{
    return m_Values.contains(id);
}

bool cli::Arguments::has_value_and_is(
    const std::string &id,
    const std::string &value) const
{
    return m_Values.contains(id) && m_Values.at(id) == value;
}

bool cli::Arguments::has_value_and_is_not(
    const std::string &id,
    const std::string &value) const
{
    return m_Values.contains(id) && m_Values.at(id) != value;
}
