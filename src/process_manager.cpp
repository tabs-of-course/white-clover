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
              << " path: " << config.executable_path 
              << " window_sequence: " << config.window_sequence << "\n";

    // Get base list of windows before launch
    auto base_windows = getAllWindows();
    std::cout << "Base window count: " << base_windows.size() << "\n";

    // Initialize process structures
    STARTUPINFOW si = {};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION pi = {};

    // Convert executable path to wide string
    std::filesystem::path exePath(config.executable_path);
    std::wstring wide_path = exePath.wstring();
    std::wstring workingDir = exePath.parent_path().wstring();

    // Create command line including arguments
    std::string cmd_line = "\"" + config.executable_path + "\"";
    for (const auto& arg : config.args) {
        cmd_line += " \"" + arg + "\"";
    }
    std::cout << "Command line: " << cmd_line << "\n"
              << "Working directory: " << exePath.parent_path().string() << "\n";
    std::wstring wide_cmd_line(cmd_line.begin(), cmd_line.end());

    // Launch the process
    if (!CreateProcessW(
            wide_path.c_str(),                    // Application name
            wide_cmd_line.empty() ? nullptr : &wide_cmd_line[0],  // Command line
            nullptr,                              // Process security attributes
            nullptr,                              // Thread security attributes
            FALSE,                                // Inherit handles
            CREATE_DEFAULT_ERROR_MODE,            // Creation flags
            nullptr,                              // Environment
            workingDir.c_str(),                   // Working directory
            &si,                                  // Startup info
            &pi)) {                               // Process info
        DWORD error = GetLastError();
        std::cout << "Failed to create process. Error code: " << error 
                  << " (0x" << std::hex << error << std::dec << ")\n";
        return false;
    }

    std::cout << "Process created successfully with:\n"
              << "  Process ID: " << pi.dwProcessId << "\n"
              << "  Thread ID: " << pi.dwThreadId << "\n"
              << "  Working Dir: " << exePath.parent_path().string() << "\n";

    // Check if process is still running
    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
        std::cout << "Process terminated immediately with exit code: " 
                  << exitCode << " (0x" << std::hex << exitCode << std::dec << ")\n";
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }
    
    // Wait for process to initialize
    std::cout << "Waiting for process to initialize...\n";
    DWORD waitResult = WaitForInputIdle(pi.hProcess, 5000);
    if (waitResult == WAIT_FAILED) {
        DWORD error = GetLastError();
        std::cout << "WaitForInputIdle failed. Error: " << error 
                  << " (0x" << std::hex << error << std::dec << ")\n";
    } else if (waitResult == WAIT_TIMEOUT) {
        std::cout << "WaitForInputIdle timed out after 5 seconds\n";
    } else {
        std::cout << "Process initialized successfully\n";
    }

    HWND target_window = nullptr;
    
    for (int sequence = 1; sequence <= config.window_sequence; sequence++) {
        bool window_found = false;
        std::cout << "Looking for window " << sequence << " in sequence\n";
        
        for (int attempt = 0; attempt < 60 && !window_found; attempt++) {
            if (attempt % 5 == 0) {
                std::cout << "Searching for window sequence " << sequence << ", attempt " << (attempt + 1) << "\n";
            }
            
            auto current_windows = getAllWindows();
            std::cout << "Current window count: " << current_windows.size() << "\n";
            
            // For sequence 3, let's log ALL windows to see what's happening
            if (sequence == 3) {
                std::cout << "Current windows during sequence 3:\n";
                for (const auto& win : current_windows) {
                    std::cout << "Window:\n"
                            << "  Handle: 0x" << std::hex << (uintptr_t)win.handle << std::dec << "\n"
                            << "  Title: " << win.title << "\n"
                            << "  Class: " << win.class_name << "\n";
                }
                std::cout << "\nBase windows for comparison:\n";
                for (const auto& base : base_windows) {
                    std::cout << "Base Window:\n"
                            << "  Handle: 0x" << std::hex << (uintptr_t)base.handle << std::dec << "\n"
                            << "  Title: " << base.title << "\n"
                            << "  Class: " << base.class_name << "\n";
                }
            }
            
            // Find the one window that's not in our base list
            for (const auto& current : current_windows) {
                bool is_new = true;
                
                // Check against base windows
                for (const auto& base : base_windows) {
                    if (current.handle == base.handle) {
                        is_new = false;
                        break;
                    }
                }
                
                if (is_new) {
                    std::cout << "Found sequence " << sequence << " window:\n"
                            << "  Handle: 0x" << std::hex << (uintptr_t)current.handle << std::dec << "\n"
                            << "  Title: " << current.title << "\n"
                            << "  Class: " << current.class_name << "\n";

                    window_found = true;

                    if (sequence < config.window_sequence) {
                        std::cout << "Sending Enter key to window " << sequence << "\n";
                        Sleep(500);
                        PostMessage(current.handle, WM_KEYDOWN, VK_RETURN, 0x001C0001);
                        Sleep(100);
                        PostMessage(current.handle, WM_KEYUP, VK_RETURN, 0xC01C0001);
                        Sleep(5000);
                    }
                    
                    if (sequence == config.window_sequence) {
                        target_window = current.handle;
                    }
                    
                    break;
                }
            }
            
            if (!window_found) {
                Sleep(13500);
            }
        }
        
        if (!window_found) {
            std::cout << "Failed to find window " << sequence << " in sequence after all attempts\n";
            return false;
        }
    }

    if (!target_window) {
        std::cerr << "Failed to get target window\n";
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
            std::cout << "Final process state - Exit code: " 
                      << exitCode << " (0x" << std::hex << exitCode << std::dec << ")\n";
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    // Store the process information with the final window handle
    ProcessInstance instance{
        pi.hProcess,
        target_window,
        config.id,
        instance_num
    };
    process_instances.push_back(instance);

    CloseHandle(pi.hThread);

    std::cout << "Successfully launched " << config.id 
              << " instance " << instance_num 
              << " (HWND: 0x" << std::hex << (uintptr_t)target_window << std::dec << ")" << std::endl;
    return true;
}

BOOL CALLBACK ProcessManager::enumWindowCallback(HWND handle, LPARAM param) {
    auto* args = reinterpret_cast<EnumWindowsCallbackArgs*>(param);

    // Check if window is visible
    BOOL is_visible = IsWindowVisible(handle);
    if (!is_visible) {
        return TRUE;
    }

    // Get window styles
    LONG styles = GetWindowLong(handle, GWL_STYLE);
    
    // Accept either:
    // 1. Windows with captions (regular windows)
    // 2. Borderless windows (typically fullscreen games)
    BOOL has_caption = (styles & WS_CAPTION) != 0;
    BOOL is_borderless = (styles & WS_POPUP) != 0;  // Borderless windows often use WS_POPUP
    
    if (!has_caption && !is_borderless) {
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