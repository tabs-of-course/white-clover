#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <filesystem>

struct ProcessConfig {
    std::string id;
    std::string executable_path;
    std::vector<std::string> args;
    int instances;
    int window_sequence;  
};

struct KeyBinding {
    std::string key;
    std::string target_process;
    std::optional<int> instance;
};



class SettingsManager {
public:
    static SettingsManager& getInstance() {
        static SettingsManager instance;
        return instance;
    }

    bool initialize();
    const std::vector<ProcessConfig>& getProcessConfigs() const { return process_configs; }
    const std::vector<KeyBinding>& getKeyBindings() const { return key_bindings; }
    void printSettings() const;

private:
    SettingsManager() = default;
    bool loadSettings(const std::filesystem::path& filepath);
    std::filesystem::path getSettingsPath() const;
    
    std::vector<ProcessConfig> process_configs;
    std::vector<KeyBinding> key_bindings;
};