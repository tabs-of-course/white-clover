#pragma once
#include <atomic>
#include <memory>
#include "message_channel.h"
#include "i_receiver.h"

class receiver : public i_receiver {
public:
    receiver(std::shared_ptr<message_channel> channel, std::atomic<bool>& running);
    void operator()() override;
    std::optional<message> receive_message() override;
    std::vector<message> receive_batch(size_t max_messages) override;

private:
    std::shared_ptr<message_channel> channel;
    std::atomic<bool>& running;
    static constexpr size_t BATCH_SIZE = 1000;
};