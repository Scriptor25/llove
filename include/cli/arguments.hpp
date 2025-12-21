#pragma once

#include <cli/templates.hpp>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace cli
{
    template<typename T>
    bool convert_value(
        T& dst,
        const std::string& value);

    class Arguments final
    {
    public:
        explicit Arguments(
            const char* const* begin,
            const char* const* end);

        std::string BuildCommandLine() const;

        const std::map<
            std::string,
            OptionTemplate>&
        templates() const;

        const std::string& filename() const;

        bool has_none_except(const std::set<std::string>& id_set) const;

        bool flag(const std::string& id) const;
        std::optional<std::string> value(const std::string& id) const;
        std::optional<std::vector<std::string>> array(const std::string& id) const;

        bool value(
            const std::string& id,
            std::string& dst) const;
        bool array(
            const std::string& id,
            std::vector<std::string>& dst) const;
        bool set(
            const std::string& id,
            std::set<std::string>& dst) const;

        bool has_value(const std::string& id) const;
        bool has_value_and_is(
            const std::string& id,
            const std::string& value) const;
        bool has_value_and_is_not(
            const std::string& id,
            const std::string& value) const;

        template<typename T>
        bool value(
            const std::string& id,
            T& dst) const
        {
            if (!m_Values.contains(id))
                return false;

            return cli::convert_value<T>(dst, m_Values.at(id));
        }

        template<typename T>
        bool array(
            const std::string& id,
            std::vector<T>& dst) const
        {
            if (!m_Arrays.contains(id))
                return false;

            for (auto& entry : m_Arrays.at(id))
                if (!cli::convert_value<T>(dst.emplace_back(), entry))
                    return false;
            return true;
        }

        template<typename T>
        bool set(
            const std::string& id,
            std::set<T>& dst) const
        {
            if (!m_Arrays.contains(id))
                return false;

            for (auto& entry : m_Arrays.at(id))
            {
                T value;
                if (!cli::convert_value<T>(value, entry))
                    return false;
                dst.insert(value);
            }
            return true;
        }

    private:
        std::map<std::string, OptionTemplate> m_Templates;

        std::string m_Filename;

        std::set<std::string> m_Flags;
        std::map<std::string, std::string> m_Values;
        std::map<std::string, std::vector<std::string>> m_Arrays;
    };
}
