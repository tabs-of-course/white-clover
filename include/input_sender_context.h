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
#include <unordered_map>

class input_sender_context : public i_thread_context {
public:
    input_sender_context(std::shared_ptr<message_channel> outbound_channel,
                        std::shared_ptr<message_channel> inbound_channel,
                        std::atomic<bool>& running,
                        HWND target_window,
                        const std::string& process_id,
                        int instance_num);

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
    std::atomic<size_t> inputs_sent{0};
    HWND target_hwnd;
    std::string process_id;      // Added to store process ID
    int instance_number;         // Added to store instance number

    // Helper functions
    void send_key_to_window(const std::string& key_name);
    WORD get_virtual_key_code(const std::string& key_name);
    void simulate_key_press(WORD vk_code);
    void simulate_key_combination(const std::vector<WORD>& vk_codes);
};