#pragma once

class i_thread_manager {
public:
    virtual ~i_thread_manager() = default;
    virtual void start_threads() = 0;
    virtual void stop_threads() = 0;
    virtual void print_metrics() const = 0;
};