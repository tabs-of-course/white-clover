#include "thread_manager.h"
#include "settings_manager.h"
#include "process_manager.h"
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    try {
        std::cout << "Starting White Clover...\n";
        
        // Initialize settings
        auto& settings = SettingsManager::getInstance();
        if (!settings.initialize()) {
            std::cerr << "Failed to initialize settings. Exiting.\n";
            return 1;
        }

        // Print loaded settings for verification
        settings.printSettings();
        
        // Launch processes
        std::cout << "Launching processes...\n";
        auto& process_mgr = ProcessManager::getInstance();
        if (!process_mgr.launch_processes()) {
            std::cerr << "Failed to launch processes. Exiting.\n";
            return 1;
        }
        
        // Create and start thread manager
        std::cout << "Creating thread manager...\n";
        thread_manager manager;
        
        // Add input sender contexts for each process
        for (const auto& proc_config : settings.getProcessConfigs()) {
            for (int i = 0; i < proc_config.instances; ++i) {
                std::cout << "Adding input sender context for " << proc_config.id 
                         << " instance " << i << "\n";
                if (!manager.add_input_sender_context(proc_config.id, i)) {
                    std::cerr << "Failed to add input sender context for " 
                             << proc_config.id << " instance " << i << "\n";
                }
            }
        }
        
        std::cout << "Starting threads...\n";
        manager.start_threads();
        
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
                std::cout << "ESC pressed, exiting..." << std::endl;
                break;
            }
        }
        
        // Cleanup
        process_mgr.terminate_processes();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Caught exception in main: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Caught unknown exception in main" << std::endl;
        return 1;
    }
}