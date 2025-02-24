#pragma once
#include <Windows.h>
#include <string>

class i_process_manager {
public:
    virtual ~i_process_manager() = default;
    virtual bool launch_processes() = 0;
    virtual HWND get_window_handle(const std::string& process_id, int instance = 0) const = 0;
    virtual void terminate_processes() = 0;
};