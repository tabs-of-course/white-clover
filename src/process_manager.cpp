#include "process_manager.h"
#include <iostream>
#include <sstream>
#include <filesystem>

bool ProcessManager::launch_processes() {
    const auto& configs = SettingsManager::getInstance().getProcessConfigs();
    
    // First try to attach to existing windows
    std::cout << "Scanning for existing windows...\n";
    bool all_found = scan_and_attach_windows();
    
    if (all_found) {
        std::cout << "All required windows found and attached.\n";
        return true;
    }
    
    std::cout << "Some windows were not found, checking auto-launch settings...\n";
    
    // Track which processes still need instances
    std::unordered_map<std::string, int> remaining_instances;
    for (const auto& config : configs) {
        int found = 0;
        for (const auto& instance : process_instances) {
            if (instance.id == config.id) {
                found++;
            }
        }
        if (found < config.instances) {
            remaining_instances[config.id] = config.instances - found;
        }
    }
    
    // Launch remaining instances if auto_launch is enabled
    bool launch_success = true;
    for (const auto& config : configs) {
        if (remaining_instances.count(config.id) > 0 && config.auto_launch) {
            std::cout << "Auto-launching " << remaining_instances[config.id] 
                      << " instances of " << config.id << "\n";
                      
            int current_instances = 0;
            for (const auto& instance : process_instances) {
                if (instance.id == config.id) {
                    current_instances = std::max(current_instances, instance.instance_number + 1);
                }
            }
            
            for (int i = 0; i < remaining_instances[config.id]; ++i) {
                if (!launchProcess(config, current_instances + i)) {
                    launch_success = false;
                    std::cerr << "Failed to launch process " << config.id 
                             << " instance " << (current_instances + i) << "\n";
                }
            }
        }
    }
    
    return launch_success;
}

bool ProcessManager::scan_and_attach_windows() {
    process_instances.clear();
    const auto& configs = SettingsManager::getInstance().getProcessConfigs();
    
    // Map to track instance counts for each process ID
    std::unordered_map<std::string, int> instance_counts;
    
    // Get all visible windows
    std::vector<WindowInfo> windows;
    EnumWindowsCallbackArgs args{windows, configs, instance_counts};
    EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(&args));
    
    // Check if we found all required instances
    bool all_found = true;
    for (const auto& config : configs) {
        int found_instances = instance_counts[config.id];
        std::cout << "Found " << found_instances << " instances for ID " << config.id << "\n";
        
        if (found_instances < config.instances) {
            std::cout << "Missing " << (config.instances - found_instances) 
                      << " instances of " << config.id << "\n";
            all_found = false;
        }
    }
    
    return all_found;
}

BOOL CALLBACK ProcessManager::enumWindowCallback(HWND handle, LPARAM param) {
    auto* args = reinterpret_cast<EnumWindowsCallbackArgs*>(param);
    
    // Check if window is visible
    if (!IsWindowVisible(handle)) {
        return TRUE;
    }
    
    // Get window title and class
    char title[256] = "";
    char class_name[256] = "";
    GetWindowTextA(handle, title, sizeof(title));
    GetClassNameA(handle, class_name, sizeof(class_name));
    
    std::string window_title(title);
    
    // Skip empty titles
    if (window_title.empty()) {
        return TRUE;
    }
    
    // Check each process configuration
    for (const auto& config : args->configs) {
        // Check if window title contains the process ID
        if (window_title.find(config.id) != std::string::npos) {
            // Get current instance count
            int current_instance = args->instance_counts[config.id]++;
            
            // Only store if we haven't exceeded desired instances
            if (current_instance < config.instances) {
                WindowInfo info{
                    handle,
                    window_title,
                    class_name
                };
                args->windows.push_back(info);
                
                // Store the process instance
                ProcessInstance instance{
                    nullptr,  // No process handle for attached windows
                    handle,
                    config.id,
                    current_instance,
                    window_title
                };
                ProcessManager::getInstance().process_instances.push_back(instance);
                
                std::cout << "Found window for " << config.id 
                          << " instance " << current_instance 
                          << ": " << window_title << "\n";
            }
        }
    }
    
    return TRUE;
}

bool ProcessManager::launchProcess(const ProcessConfig& config, int instance_num) {
    std::cout << "Launching process: " << config.id 
              << " instance " << instance_num 
              << " path: " << config.executable_path 
              << " window_sequence: " << config.window_sequence << "\n";

    auto base_windows = getAllWindows();

    STARTUPINFOW si = {};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION pi = {};

    std::filesystem::path exePath(config.executable_path);
    std::wstring wide_path = exePath.wstring();
    std::wstring workingDir = exePath.parent_path().wstring();

    std::string cmd_line = "\"" + config.executable_path + "\"";
    for (const auto& arg : config.args) {
        cmd_line += " \"" + arg + "\"";
    }
    std::wstring wide_cmd_line(cmd_line.begin(), cmd_line.end());

    if (!CreateProcessW(
            wide_path.c_str(),
            wide_cmd_line.empty() ? nullptr : &wide_cmd_line[0],
            nullptr,
            nullptr,
            FALSE,
            CREATE_DEFAULT_ERROR_MODE,
            nullptr,
            workingDir.c_str(),
            &si,
            &pi)) {
        DWORD error = GetLastError();
        std::cerr << "Failed to create process. Error: " << error << "\n";
        return false;
    }

    // Wait for process to initialize
    WaitForInputIdle(pi.hProcess, 5000);

    HWND target_window = nullptr;
    
    for (int sequence = 1; sequence <= config.window_sequence; sequence++) {
        bool window_found = false;
        
        for (int attempt = 0; attempt < 60 && !window_found; attempt++) {
            if (attempt % 5 == 0) {
                std::cout << "Searching for window sequence " << sequence 
                          << ", attempt " << (attempt + 1) << "\n";
            }
            
            target_window = findNewWindow(base_windows);
            if (target_window) {
                window_found = true;
                
                if (sequence < config.window_sequence) {
                    std::cout << "Sending Enter key to window " << sequence << "\n";
                    Sleep(500);
                    PostMessage(target_window, WM_KEYDOWN, VK_RETURN, 0x001C0001);
                    Sleep(100);
                    PostMessage(target_window, WM_KEYUP, VK_RETURN, 0xC01C0001);
                    Sleep(2000);
                }
                
                // Update base windows for next sequence
                base_windows = getAllWindows();
            }
            
            if (!window_found) {
                Sleep(1000);
            }
        }
        
        if (!window_found) {
            std::cerr << "Failed to find window " << sequence << " in sequence\n";
            TerminateProcess(pi.hProcess, 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return false;
        }
    }

    if (!target_window) {
        std::cerr << "Failed to get target window\n";
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    // Store the process information
    ProcessInstance instance{
        pi.hProcess,
        target_window,
        config.id,
        instance_num,
        ""  // Window title will be fetched when needed
    };
    
    // Get the window title
    char title[256] = "";
    GetWindowTextA(target_window, title, sizeof(title));
    instance.window_title = title;
    
    process_instances.push_back(instance);

    CloseHandle(pi.hThread);
    return true;
}

std::vector<WindowInfo> ProcessManager::getAllWindows() const {
    std::vector<WindowInfo> windows;
    EnumWindows(enumWindowCallbackBasic, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

BOOL CALLBACK ProcessManager::enumWindowCallbackBasic(HWND handle, LPARAM param) {
    auto* windows = reinterpret_cast<std::vector<WindowInfo>*>(param);
    
    if (!IsWindowVisible(handle)) {
        return TRUE;
    }
    
    char title[256] = "";
    char class_name[256] = "";
    GetWindowTextA(handle, title, sizeof(title));
    GetClassNameA(handle, class_name, sizeof(class_name));
    
    WindowInfo info{
        handle,
        title,
        class_name
    };
    
    windows->push_back(info);
    return TRUE;
}

HWND ProcessManager::findNewWindow(const std::vector<WindowInfo>& before_windows) const {
    auto current_windows = getAllWindows();
    
    for (const auto& current : current_windows) {
        bool is_new = true;
        for (const auto& before : before_windows) {
            if (current.handle == before.handle) {
                is_new = false;
                break;
            }
        }
        
        if (is_new && !current.title.empty()) {
            std::cout << "Found new window: " << current.title << "\n";
            return current.handle;
        }
    }
    
    return nullptr;
}

HWND ProcessManager::get_window_handle(const std::string& process_id, int instance) const {
    for (const auto& proc : process_instances) {
        if (proc.id == process_id && proc.instance_number == instance) {
            return proc.window_handle;
        }
    }
    return nullptr;
}

void ProcessManager::terminate_processes() {
    for (const auto& instance : process_instances) {
        if (instance.process_handle) {  // Only terminate processes we launched
            TerminateProcess(instance.process_handle, 0);
            CloseHandle(instance.process_handle);
        }
    }
    process_instances.clear();
}