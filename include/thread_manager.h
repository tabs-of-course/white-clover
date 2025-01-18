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
    
    // Add getter for input channels
    static std::shared_ptr<message_channel> get_input_channel(const std::string& context_id) {
        auto it = input_channels.find(context_id);
        return (it != input_channels.end()) ? it->second : nullptr;
    }

private:
    std::unique_ptr<i_thread_context> key_monitor_context;
    std::shared_ptr<message_channel> key_monitor_outbound;
    std::unordered_map<std::string, ContextInfo> input_contexts;
    static std::unordered_map<std::string, std::shared_ptr<message_channel>> input_channels;
    std::atomic<bool> running{true};
};