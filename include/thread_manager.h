#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include "message_channel.h"
#include "sender.h"
#include "receiver.h"
#include "i_thread_manager.h"

class thread_manager : public i_thread_manager {
public:
    thread_manager();
    ~thread_manager();
    
    void start_threads() override;
    void stop_threads() override;
    void print_metrics() const override;

private:
    std::shared_ptr<message_channel> channel;
    std::atomic<bool> running{true};
    std::thread sender_thread;
    std::thread receiver_thread;
    sender msg_sender;
    receiver msg_receiver;
};