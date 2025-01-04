#pragma once
#include "i_thread_context.h"
#include "message_channel.h"
#include "sender.h"
#include "receiver.h"
#include <Windows.h>
#include <memory>
#include <atomic>
#include <thread>
#include <string>

class key_monitor_context : public i_thread_context {
public:
    key_monitor_context(std::shared_ptr<message_channel> outbound_channel,
                       std::shared_ptr<message_channel> inbound_channel,
                       std::atomic<bool>& running);

    void operator()() override;
    void start() override;
    void stop() override;
    void process_message(const message& msg) override;
    void print_metrics() const override;
    void set_name(const std::string& name) override;

private:
    std::shared_ptr<message_channel> outbound_channel;
    std::shared_ptr<message_channel> inbound_channel;
    std::atomic<bool>& running;
    std::thread worker_thread;
    sender msg_sender;
    receiver msg_receiver;
    std::string context_name;
    std::atomic<size_t> messages_processed{0};
    std::atomic<size_t> messages_sent{0};
    std::atomic<size_t> keys_processed{0};

    // Helper function to convert virtual key code to string
    std::string get_key_name(DWORD vk_code);
};