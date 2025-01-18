#include "process_manager.h"
#include "settings_manager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

bool ProcessManager::launch_processes() {
    const auto& configs = SettingsManager::getInstance().getProcessConfigs();
    bool all_success = true;

    for (const auto& config : configs) {
        for (int i = 0; i < config.instances; ++i) {
            if (!launchProcess(config, i)) {
                all_success = false;
                std::cerr << "Failed to launch process " << config.id 
                         << " instance " << i << std::endl;
            }
        }
    }

    return all_success;
}

std::vector<WindowInfo> ProcessManager::getAllWindows() const {
    std::vector<WindowInfo> windows;
    EnumWindowsCallbackArgs args{windows};
    EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(&args));
    return windows;
}

HWND ProcessManager::findNewWindow(const std::vector<WindowInfo>& before_windows) const {
    // Get current windows
    std::vector<WindowInfo> current_windows = getAllWindows();

    std::cout << "Windows found before launch: " << before_windows.size() << "\n";
    std::cout << "Windows found after launch: " << current_windows.size() << "\n";

    // Look for a window that wasn't in the before_windows list
    for (const auto& current : current_windows) {
        bool is_new = true;
        for (const auto& before : before_windows) {
            if (current.handle == before.handle) {
                is_new = false;
                break;
            }
        }
        
        if (is_new) {
            std::cout << "Found new window:\n"
                      << "  Handle: 0x" << std::hex << (uintptr_t)current.handle << std::dec << "\n"
                      << "  Title: " << current.title << "\n"
                      << "  Class: " << current.class_name << "\n";
            return current.handle;
        }
    }

    return nullptr;
}

bool ProcessManager::launchProcess(const ProcessConfig& config, int instance_num) {
    std::cout << "Launching process: " << config.id 
              << " instance " << instance_num 
              << " path: " << config.executable_path << "\n";

    // Get list of windows before launch
    auto before_windows = getAllWindows();

    // Initialize process structures
    STARTUPINFOW si = {};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION pi = {
        NULL,               // hProcess
        NULL,               // hThread
        0,                  // dwProcessId
        0                   // dwThreadId
    };

    // Convert executable path and arguments to wide string
    std::wstring wide_path(config.executable_path.begin(), config.executable_path.end());
    
    // Create command line including arguments
    std::string cmd_line = config.executable_path;
    for (const auto& arg : config.args) {
        cmd_line += " " + arg;
    }
    std::cout << "Command line: " << cmd_line << "\n";
    std::wstring wide_cmd_line(cmd_line.begin(), cmd_line.end());

    // Launch the process
    if (!CreateProcessW(
            wide_path.c_str(),
            wide_cmd_line.empty() ? nullptr : &wide_cmd_line[0],
            nullptr, nullptr, FALSE, 0, nullptr, nullptr,
            &si, &pi)) {
        std::cout << "Failed to create process. Error: " << GetLastError() << "\n";
        return false;
    }

    std::cout << "Process created successfully!\n";
    
    // Wait for process to initialize
    WaitForInputIdle(pi.hProcess, 5000);

    // Try to find the new window
    HWND hwnd = nullptr;
    for (int attempt = 0; attempt < 10 && !hwnd; attempt++) {
        std::cout << "Searching for new window, attempt " << (attempt + 1) << "\n";
        hwnd = findNewWindow(before_windows);
        if (!hwnd) {
            Sleep(500);
        }
    }

    if (!hwnd) {
        std::cerr << "Failed to find new window\n";
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    // Store the process information
    ProcessInstance instance{
        pi.hProcess,
        hwnd,
        config.id,
        instance_num
    };
    process_instances.push_back(instance);

    CloseHandle(pi.hThread);

    std::cout << "Successfully launched " << config.id 
              << " instance " << instance_num 
              << " (HWND: 0x" << std::hex << (uintptr_t)hwnd << std::dec << ")" << std::endl;
    return true;
}

BOOL CALLBACK ProcessManager::enumWindowCallback(HWND handle, LPARAM param) {
    auto* args = reinterpret_cast<EnumWindowsCallbackArgs*>(param);

    // Check if window is visible and has a caption
    BOOL is_visible = IsWindowVisible(handle);
    LONG styles = GetWindowLong(handle, GWL_STYLE);
    BOOL has_caption = (styles & WS_CAPTION) != 0;

    if (!is_visible || !has_caption) {
        return TRUE;
    }

    // Get window info
    char title[256] = "";
    char class_name[256] = "";
    GetWindowTextA(handle, title, sizeof(title));
    GetClassNameA(handle, class_name, sizeof(class_name));

    // Store window info
    WindowInfo info{
        handle,
        title,
        class_name
    };

    args->windows.push_back(info);
    return TRUE;
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
        TerminateProcess(instance.process_handle, 0);
        CloseHandle(instance.process_handle);
    }
    process_instances.clear();
}