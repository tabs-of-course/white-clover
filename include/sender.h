#pragma once
#include <atomic>
#include <memory>
#include "message_channel.h"
#include "i_sender.h"

class sender : public i_sender {
public:
    sender(std::shared_ptr<message_channel> channel, std::atomic<bool>& running);
    void operator()() override;
    bool send_message(const message& msg) override;
    bool send_batch(const std::vector<message>& messages) override;

private:
    std::shared_ptr<message_channel> channel;
    std::atomic<bool>& running;
    static constexpr size_t BATCH_SIZE = 1000;
};