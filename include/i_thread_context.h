#pragma once
#include "message_types.h"
#include <string>

class i_thread_context {
public:
    virtual ~i_thread_context() = default;
    
    virtual void operator()() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void process_message(const message& msg) = 0;
    virtual void print_metrics() const = 0;
    virtual void set_name(const std::string& name) = 0;
};