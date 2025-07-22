#pragma once

#include <cli/templates.hpp>
#include <llove/error.hpp>
#include <yaml-cpp/yaml.h>

namespace cli
{
    std::string GetConfig();
    std::vector<std::string> SplitString(std::string value, char delimiter);

    class Arguments final
    {
    public:
        explicit Arguments(const char *const *begin, const char *const *end);
        [[nodiscard]] const std::map<std::string, OptionTemplate> &templates() const;
        [[nodiscard]] bool flag(const std::string &name) const;
        [[nodiscard]] std::optional<std::string> value(const std::string &name) const;
        [[nodiscard]] std::optional<std::vector<std::string>> array(const std::string &name) const;
        [[nodiscard]] bool value(const std::string &name, std::string &dst) const;
        [[nodiscard]] bool array(const std::string &name, std::vector<std::string> &dst) const;
        [[nodiscard]] const std::string &filename() const;

    private:
        std::map<std::string, OptionTemplate> m_Templates;

        std::string m_Filename;
        std::set<std::string> m_Flags;
        std::map<std::string, std::string> m_Values;
        std::map<std::string, std::vector<std::string>> m_Arrays;
    };
}
