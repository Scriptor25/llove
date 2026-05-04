#pragma once

#include <cli/templates.hpp>

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Target/TargetOptions.h>

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace cli
{
    template<typename T>
    bool convert_value(T &dst, const std::string &value) = delete;

    template<>
    bool convert_value(int &dst, const std::string &value);
    template<>
    bool convert_value(unsigned &dst, const std::string &value);
    template<>
    bool convert_value(std::filesystem::path &dst, const std::string &value);
    template<>
    bool convert_value(llvm::DICompileUnit::DebugEmissionKind &dst, const std::string &value);
    template<>
    bool convert_value(llvm::CodeGenFileType &dst, const std::string &value);
    template<>
    bool convert_value(llvm::Reloc::Model &dst, const std::string &value);
    template<>
    bool convert_value(llvm::OptimizationLevel &dst, const std::string &value);
    template<>
    bool convert_value(llvm::GlobalISelAbortMode &dst, const std::string &value);
    template<>
    bool convert_value(llvm::SwiftAsyncFramePointerMode &dst, const std::string &value);
    template<>
    bool convert_value(llvm::DebugCompressionType &dst, const std::string &value);
    template<>
    bool convert_value(llvm::BasicBlockSection &dst, const std::string &value);
    template<>
    bool convert_value(llvm::FloatABI::ABIType &dst, const std::string &value);
    template<>
    bool convert_value(llvm::FPOpFusion::FPOpFusionMode &dst, const std::string &value);
    template<>
    bool convert_value(llvm::ThreadModel::Model &dst, const std::string &value);
    template<>
    bool convert_value(llvm::EABI &dst, const std::string &value);
    template<>
    bool convert_value(llvm::DebuggerKind &dst, const std::string &value);
    template<>
    bool convert_value(llvm::DenormalMode::DenormalModeKind &dst, const std::string &value);
    template<>
    bool convert_value(llvm::ExceptionHandling &dst, const std::string &value);
    template<>
    bool convert_value(llvm::EmitDwarfUnwindType &dst, const std::string &value);
    template<>
    bool convert_value(llvm::MCTargetOptions::DwarfDirectory &dst, const std::string &value);
    template<>
    bool convert_value(llvm::VectorLibrary &dst, const std::string &value);

    class Arguments final
    {
    public:
        explicit Arguments(
            const char *const*begin,
            const char *const*end);

        [[nodiscard]] std::string BuildCommandLine() const;

        [[nodiscard]] const std::map<std::string, OptionTemplate> &templates() const;

        [[nodiscard]] const std::string &filename() const;

        [[nodiscard]] bool has_none_except(const std::set<std::string> &id_set) const;

        [[nodiscard]] bool flag(const std::string &id) const;
        [[nodiscard]] std::optional<std::string> value(const std::string &id) const;
        [[nodiscard]] std::optional<std::vector<std::string>> array(const std::string &id) const;

        bool value(const std::string &id, std::string &dst) const;
        bool array(const std::string &id, std::vector<std::string> &dst) const;
        bool set(const std::string &id, std::set<std::string> &dst) const;

        [[nodiscard]] bool has_value(const std::string &id) const;
        [[nodiscard]] bool has_value_and_is(const std::string &id, const std::string &value) const;
        [[nodiscard]] bool has_value_and_is_not(const std::string &id, const std::string &value) const;

        template<typename T>
        bool value(const std::string &id, T &dst) const
        {
            if (const auto it = m_Values.find(id); it != m_Values.end())
                return cli::convert_value<T>(dst, it->second);
            return false;
        }

        template<typename T>
        bool array(const std::string &id, std::vector<T> &dst) const
        {
            if (!m_Arrays.contains(id))
                return false;

            for (auto &entry : m_Arrays.at(id))
                if (!cli::convert_value<T>(dst.emplace_back(), entry))
                    return false;
            return true;
        }

        template<typename T>
        bool set(const std::string &id, std::set<T> &dst) const
        {
            if (!m_Arrays.contains(id))
                return false;

            for (auto &entry : m_Arrays.at(id))
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
