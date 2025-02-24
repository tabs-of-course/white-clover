#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "settings_manager.h"
#include "i_process_manager.h"

struct ProcessInstance {
    HANDLE process_handle;    // Will be nullptr for attached windows
    HWND window_handle;
    std::string id;
    int instance_number;
    std::string window_title;
};

struct WindowInfo {
    HWND handle;
    std::string title;
    std::string class_name;
};

struct EnumWindowsCallbackArgs {
    std::vector<WindowInfo>& windows;
    const std::vector<ProcessConfig>& configs;
    std::unordered_map<std::string, int>& instance_counts;
};

class ProcessManager : public i_process_manager {
public:
    static ProcessManager& getInstance() {
        static ProcessManager instance;
        return instance;
    }

    bool launch_processes() override;
    HWND get_window_handle(const std::string& process_id, int instance = 0) const override;
    void terminate_processes() override;

private:
    ProcessManager() = default;
    ~ProcessManager() = default;
    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;

    // Window scanning functions
    bool scan_and_attach_windows();
    static BOOL CALLBACK enumWindowCallback(HWND handle, LPARAM param);
    std::vector<WindowInfo> getAllWindows() const;

    // Process launching functions
    bool launchProcess(const ProcessConfig& config, int instance_num);
    HWND findNewWindow(const std::vector<WindowInfo>& before_windows) const;
    static BOOL CALLBACK enumWindowCallbackBasic(HWND handle, LPARAM param);
    
    std::vector<ProcessInstance> process_instances;
};