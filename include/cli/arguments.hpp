#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <cli/templates.hpp>
#include <yaml-cpp/yaml.h>

namespace cli
{
    class Arguments final
    {
    public:
        explicit Arguments(const char *const *begin, const char *const *end);

        [[nodiscard]] std::string BuildCommandLine() const;

        [[nodiscard]] const std::map<std::string, OptionTemplate> &templates() const;

        [[nodiscard]] const std::string &filename() const;

        [[nodiscard]] bool has_none_except(const std::set<std::string> &id_set) const;

        [[nodiscard]] bool flag(const std::string &id) const;
        [[nodiscard]] std::optional<std::string> value(const std::string &id) const;
        [[nodiscard]] std::optional<std::vector<std::string>> array(const std::string &id) const;

        [[nodiscard]] bool value(const std::string &id, std::string &dst) const;
        [[nodiscard]] bool array(const std::string &id, std::vector<std::string> &dst) const;

        [[nodiscard]] bool has_value(const std::string &id) const;
        [[nodiscard]] bool has_value_and_is(const std::string &id, const std::string &value) const;
        [[nodiscard]] bool has_value_and_is_not(const std::string &id, const std::string &value) const;

    private:
        std::map<std::string, OptionTemplate> m_Templates;

        std::string m_Filename;

        std::set<std::string> m_Flags;
        std::map<std::string, std::string> m_Values;
        std::map<std::string, std::vector<std::string>> m_Arrays;
    };
}
