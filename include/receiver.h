#pragma once
#include <atomic>
#include <memory>
#include "message_channel.h"

class receiver {
public:
    receiver(std::shared_ptr<message_channel> channel, std::atomic<bool>& running);
    void operator()();  // Thread function

private:
    std::shared_ptr<message_channel> channel;
    std::atomic<bool>& running;
    static constexpr size_t BATCH_SIZE = 1000;
};