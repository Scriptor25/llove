#include <config.h>
#include <cli/arguments.hpp>

std::string cli::GetConfig()
{
    return { reinterpret_cast<const char *>(config_data), config_data_len };
}

std::vector<std::string> cli::SplitString(std::string value, const char delimiter)
{
    std::vector<std::string> values;
    for (size_t pos; (pos = value.find(delimiter)) != std::string::npos;)
    {
        values.emplace_back(value.substr(0, pos));
        value = value.substr(pos + 1);
    }
    if (!value.empty())
        values.emplace_back(value);
    return values;
}

cli::Arguments::Arguments(const char *const *begin, const char *const *end)
{
    m_Templates = YAML::Load(GetConfig()).as<std::map<std::string, OptionTemplate>>();

    for (auto i = begin; i != end; ++i)
    {
        auto arg = SplitString(*i, '=');

        auto match = false;
        for (auto &[fst, snd] : m_Templates)
        {
            auto &pat = arg.at(0);
            if (!snd.Pattern.contains(pat))
                continue;

            match = true;
            switch (snd.Type)
            {
            case OptionTemplateType_Flag:
                llove::Assert(arg.size() == 1, "illegal use of argument '{}': must be a flag", pat);
                m_Flags.insert(fst);
                break;
            case OptionTemplateType_Value:
            {
                auto &val = arg.at(1);
                llove::Assert(
                    snd.Filter.empty() || snd.Filter.contains(val),
                    "illegal use of argument '{}': value '{}' does not match filter",
                    pat,
                    val);
                m_Values.emplace(fst, val);
                break;
            }
            case OptionTemplateType_Array:
            {
                auto vals = SplitString(arg.at(1), ',');
                for (auto &val : vals)
                    llove::Assert(
                        snd.Filter.empty() || snd.Filter.contains(val),
                        "illegal use of argument '{}': value '{}' does not match filter",
                        pat,
                        val);
                m_Arrays.emplace(fst, std::move(vals));
                break;
            }
            }
        }

        if (match)
            continue;

        llove::Assert(
            m_Filename.empty(),
            "illegal positional argument '{}': only one filename can be specified",
            *i);
        m_Filename = *i;
    }
}

const std::map<std::string, cli::OptionTemplate> &cli::Arguments::templates() const
{
    return m_Templates;
}

bool cli::Arguments::flag(const std::string &name) const
{
    return m_Flags.contains(name);
}

std::optional<std::string> cli::Arguments::value(const std::string &name) const
{
    return m_Values.contains(name) ? std::optional{ m_Values.at(name) } : std::nullopt;
}

std::optional<std::vector<std::string>> cli::Arguments::array(const std::string &name) const
{
    return m_Arrays.contains(name) ? std::optional{ m_Arrays.at(name) } : std::nullopt;
}

bool cli::Arguments::value(const std::string &name, std::string &dst) const
{
    if (!m_Values.contains(name))
        return false;
    dst = m_Values.at(name);
    return true;
}

bool cli::Arguments::array(const std::string &name, std::vector<std::string> &dst) const
{
    if (!m_Arrays.contains(name))
        return false;
    dst = m_Arrays.at(name);
    return true;
}

const std::string &cli::Arguments::filename() const
{
    return m_Filename;
}
