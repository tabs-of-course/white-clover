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
    HANDLE process_handle;
    HWND window_handle;
    std::string id;
    int instance_number;
};

struct WindowInfo {
    HWND handle;
    std::string title;
    std::string class_name;
};

struct EnumWindowsCallbackArgs {
    std::vector<WindowInfo>& windows;
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
    bool launchProcess(const ProcessConfig& config, int instance_num);
    static BOOL CALLBACK enumWindowCallback(HWND handle, LPARAM param);
    HWND findNewWindow(const std::vector<WindowInfo>& before_windows) const;
    std::vector<WindowInfo> getAllWindows() const;
    
    std::vector<ProcessInstance> process_instances;
};