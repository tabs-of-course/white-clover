#pragma once
#include <memory>
#include <atomic>
#include "thread_context.h"
#include "i_thread_manager.h"

class thread_manager : public i_thread_manager {
public:
    thread_manager();
    ~thread_manager();

    void start_threads() override;
    void stop_threads() override;
    void print_metrics() const override;

private:
    std::shared_ptr<message_channel> channel_1_to_2;  // Messages from thread 1 to 2
    std::shared_ptr<message_channel> channel_2_to_1;  // Messages from thread 2 to 1
    std::atomic<bool> running{true};
    
    std::unique_ptr<i_thread_context> context1;  // Now using interface
    std::unique_ptr<i_thread_context> context2;  // Now using interface
};