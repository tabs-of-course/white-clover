#pragma once
#include <string>

class i_thread_manager {
public:
    virtual ~i_thread_manager() = default;
    virtual void start_threads() = 0;
    virtual void stop_threads() = 0;
    virtual void print_metrics() const = 0;

    virtual bool add_input_sender_context(const std::string& process_id, int instance) = 0;
    virtual bool remove_input_sender_context(const std::string& process_id, int instance) = 0;
};