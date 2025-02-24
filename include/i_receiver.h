#pragma once
#include "message_types.h"
#include <vector>
#include <optional>

class i_receiver {
public:
    virtual ~i_receiver() = default;
    virtual void operator()() = 0;  // Main thread function
    virtual std::optional<message> receive_message() = 0;
    virtual std::vector<message> receive_batch(size_t max_messages) = 0;
};