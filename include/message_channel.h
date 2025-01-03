#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "message_types.h"

struct message_channel {
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<message> messages;
    static constexpr size_t MAX_QUEUE_SIZE = 1000;
};