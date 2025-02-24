#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <filesystem>

struct ProcessConfig {
    std::string id;                      // Identifier to match in window title
    int instances;                       // Maximum number of instances to look for
    bool auto_launch;                    // Whether to launch if window not found
    std::string executable_path;         // Path to executable (only used if auto_launch is true)
    std::vector<std::string> args;       // Launch arguments (only used if auto_launch is true)
    int window_sequence;                 // Number of windows in sequence (only used if auto_launch is true)
};

struct KeyAction {
    std::string key;
    int delay{0};  // Delay in milliseconds after this key press
};

struct KeySequence {
    std::string target_process;
    int instance{0};
    std::vector<KeyAction> actions;
};

struct KeyBinding {
    std::string trigger_key;
    std::vector<KeySequence> sequences;
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