#pragma once
#include "message_types.h"
#include <vector>

class i_sender {
public:
    virtual ~i_sender() = default;
    virtual void operator()() = 0;  // Main thread function
    virtual bool send_message(const message& msg) = 0;
    virtual bool send_batch(const std::vector<message>& messages) = 0;
};