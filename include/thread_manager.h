#pragma once
#include <memory>
#include <atomic>
#include <unordered_map>
#include "thread_context.h"
#include "i_thread_manager.h"
#include "message_channel.h"

struct ContextInfo {
    std::shared_ptr<message_channel> outbound_channel;  // Changed from channel_to_input
    std::unique_ptr<i_thread_context> context;          // Removed channel_from_input
};

class thread_manager : public i_thread_manager {
public:
    thread_manager();
    ~thread_manager();

    void start_threads() override;
    void stop_threads() override;
    void print_metrics() const override;
    
    bool add_input_sender_context(const std::string& process_id, int instance) override;
    bool remove_input_sender_context(const std::string& process_id, int instance) override;

private:
    std::unique_ptr<i_thread_context> key_monitor_context;
    std::shared_ptr<message_channel> key_monitor_outbound;  // Add this line
    std::unordered_map<std::string, ContextInfo> input_contexts;
    std::atomic<bool> running{true};
};