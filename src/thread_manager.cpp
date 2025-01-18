#include "thread_manager.h"
#include "key_monitor_context.h"
#include "input_sender_context.h"
#include "process_manager.h"
#include <iostream>
#include <sstream>

std::unordered_map<std::string, std::shared_ptr<message_channel>> thread_manager::input_channels;

thread_manager::thread_manager() {
    // Create the key monitor context with its channels
    auto monitor_outbound = std::make_shared<message_channel>();
    auto monitor_inbound = std::make_shared<message_channel>();
    
    key_monitor_outbound = monitor_outbound;  // Store for input senders to use
    
    // Create key monitor context
    auto monitor = std::make_unique<class key_monitor_context>(
        monitor_outbound,
        monitor_inbound,
        running
    );
    
    monitor->set_name("KeyMonitor");
    key_monitor_context = std::move(monitor);

    // Clear any existing channels
    input_channels.clear();
}

thread_manager::~thread_manager() {
    stop_threads();
}

void thread_manager::start_threads() {
    std::cout << "Starting threads..." << std::endl;
    
    // Start key monitor
    if (key_monitor_context) {
        key_monitor_context->start();
    }
    
    // Start all input contexts
    for (auto& [id, context_info] : input_contexts) {
        if (context_info.context) {
            context_info.context->start();
        }
    }
}

void thread_manager::stop_threads() {
    std::cout << "Stopping threads..." << std::endl;
    running = false;
    
    // Stop key monitor
    if (key_monitor_context) {
        key_monitor_context->stop();
    }
    
    // Stop all input contexts
    for (auto& [id, context_info] : input_contexts) {
        if (context_info.context) {
            context_info.context->stop();
        }
    }
    
    // Print final metrics
    print_metrics();
}

bool thread_manager::add_input_sender_context(const std::string& process_id, int instance) {
    std::ostringstream oss;
    oss << process_id << ":" << instance;
    std::string context_id = oss.str();
    
    if (input_contexts.find(context_id) != input_contexts.end()) {
        std::cerr << "Context already exists for " << context_id << std::endl;
        return false;
    }
    
    HWND hwnd = ProcessManager::getInstance().get_window_handle(process_id, instance);
    if (!hwnd) {
        std::cerr << "Failed to get window handle for " << context_id << std::endl;
        return false;
    }
    
    std::cout << "Adding input sender context for " << context_id << std::endl;
    
    // Create dedicated inbound channel for this input sender
    auto inbound_channel = std::make_shared<message_channel>();
    input_channels[context_id] = inbound_channel;  // Store for key monitor to use
    
    auto outbound = std::make_shared<message_channel>();
    
    auto input_context = std::make_unique<input_sender_context>(
        outbound,
        inbound_channel,     // Use dedicated inbound channel
        running,
        hwnd,
        process_id,
        instance
    );
    input_context->set_name("InputSender_" + context_id);
    
    ContextInfo info{
        outbound,
        std::move(input_context)
    };
    
    input_contexts[context_id] = std::move(info);
    return true;
}

bool thread_manager::remove_input_sender_context(const std::string& process_id, int instance) {
    std::ostringstream oss;
    oss << process_id << ":" << instance;
    std::string context_id = oss.str();
    
    auto it = input_contexts.find(context_id);
    if (it == input_contexts.end()) {
        std::cerr << "Context not found for " << context_id << std::endl;
        return false;
    }
    
    // Stop the context if it's running
    if (it->second.context) {
        it->second.context->stop();
    }
    
    // Remove from both maps
    input_contexts.erase(it);
    input_channels.erase(context_id);
    
    return true;
}

void thread_manager::print_metrics() const {
    std::cout << "\n=== System Metrics ===" << std::endl;
    
    if (key_monitor_context) {
        key_monitor_context->print_metrics();
    }
    
    for (const auto& [id, context_info] : input_contexts) {
        if (context_info.context) {
            context_info.context->print_metrics();
        }
    }
    
    std::cout << "==================\n" << std::endl;
}