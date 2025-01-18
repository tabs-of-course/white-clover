#include "settings_manager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <Windows.h>

namespace fs = std::filesystem;

bool SettingsManager::initialize() {
    try {
        fs::path settings_path = getSettingsPath();
        std::cout << "Looking for settings file at: " << settings_path << "\n";
        
        if (!fs::exists(settings_path)) {
            std::cerr << "Settings file not found at: " << settings_path << "\n";
            return false;
        }

        return loadSettings(settings_path);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in initialize: " << e.what() << std::endl;
        return false;
    }
}

fs::path SettingsManager::getSettingsPath() const {
    // Get the executable path
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    
    // Get the executable directory and go up two levels to get to actual root
    // From: root/build/bin/executable.exe
    // To:   root/
    fs::path exe_path(buffer);
    fs::path root_path = exe_path.parent_path()  // bin/
                                .parent_path()    // build/
                                .parent_path();   // root/
    
    std::cout << "Executable path: " << exe_path << "\n";
    std::cout << "Root path: " << root_path << "\n";
    
    // Construct the config path
    return root_path / "config" / "settings.json";
}

bool SettingsManager::loadSettings(const fs::path& filepath) {
   try {
       std::cout << "Loading settings from: " << filepath << "\n";
       std::ifstream file(filepath);
       if (!file.is_open()) {
           std::cerr << "Failed to open settings file\n";
           return false;
       }

       std::cout << "Reading JSON content...\n";
       nlohmann::json json;
       file >> json;

       // Clear existing configurations
       process_configs.clear();
       key_bindings.clear();

       // Parse process configurations
       for (const auto& proc : json["processes"]) {
           ProcessConfig config;
           config.id = proc["id"].get<std::string>();
           config.executable_path = proc["path"].get<std::string>();
           config.instances = proc["instances"].get<int>();
           config.window_sequence = proc["window_sequence"].get<int>();
           
           if (proc.contains("args")) {
               config.args = proc["args"].get<std::vector<std::string>>();
           }
           
           process_configs.push_back(config);
           std::cout << "Added process config: " << config.id 
                     << " with window_sequence: " << config.window_sequence << "\n";
       }

       // Parse key bindings
       for (const auto& binding : json["key_bindings"]) {
           KeyBinding kb;
           kb.key = binding["key"].get<std::string>();
           kb.target_process = binding["process"].get<std::string>();
           
           if (binding.contains("instance")) {
               kb.instance = binding["instance"].get<int>();
           }
           
           key_bindings.push_back(kb);
           std::cout << "Added key binding: " << kb.key << " -> " << kb.target_process << "\n";
       }

       return true;
   }
   catch (const std::exception& e) {
       std::cerr << "Error parsing settings file: " << e.what() << std::endl;
       return false;
   }
}

void SettingsManager::printSettings() const {
    std::cout << "\n=== Current Settings ===\n";
    std::cout << "Processes (" << process_configs.size() << "):\n";
    for (const auto& proc : process_configs) {
        std::cout << "  - ID: " << proc.id
                  << "\n    Path: " << proc.executable_path
                  << "\n    Instances: " << proc.instances
                  << "\n    Args: ";
        for (const auto& arg : proc.args) {
            std::cout << arg << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\nKey Bindings (" << key_bindings.size() << "):\n";
    for (const auto& kb : key_bindings) {
        std::cout << "  - Key: " << kb.key
                  << "\n    Process: " << kb.target_process
                  << "\n    Instance: " << (kb.instance ? std::to_string(*kb.instance) : "all")
                  << "\n";
    }
    std::cout << "====================\n\n";
}