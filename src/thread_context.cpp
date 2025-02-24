#include "thread_context.h"
#include <iostream>
#include <chrono>

thread_context::thread_context(std::shared_ptr<message_channel> outbound_channel,
                             std::shared_ptr<message_channel> inbound_channel,
                             std::atomic<bool>& running)
    : outbound_channel(outbound_channel)
    , inbound_channel(inbound_channel)
    , running(running)
    , msg_sender(outbound_channel, running)
    , msg_receiver(inbound_channel, running) {}

void thread_context::process_message(const message& msg) {
    std::cout << context_name << " processing - Command: " << msg.m_command 
              << " ID: " << msg.m_msg_id 
              << " Content: " << msg.m_msg << std::endl;

    // Increment processed count
    messages_processed++;

    // Send a response
    message response(msg.m_command + 1,
                    msg.m_msg_id,
                    "Response from " + context_name + " to: " + msg.m_msg);
    
    if (msg_sender.send_message(response)) {
        messages_sent++;
    }
}

void thread_context::operator()() {
    // Initial message to start the conversation
    if (context_name == "Context1") {
        message initial_msg(1, 0, "Initial message from " + context_name);
        if (msg_sender.send_message(initial_msg)) {
            messages_sent++;
        }
    }

    while (running) {
        // Check for incoming messages
        auto received = msg_receiver.receive_batch(10);
        
        for (const auto& msg : received) {
            process_message(msg);
        }

        if (received.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // print_metrics(); // Print metrics periodically when idle
        }
    }
}

void thread_context::start() {
    worker_thread = std::thread(&thread_context::operator(), this);
}

void thread_context::stop() {
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void thread_context::print_metrics() const {
    std::cout << context_name << " Metrics:"
              << " Messages Processed: " << messages_processed
              << " Messages Sent: " << messages_sent
              << " Inbound Queue: " << inbound_channel->messages.size()
              << " Outbound Queue: " << outbound_channel->messages.size()
              << std::endl;
}

void thread_context::set_name(const std::string& name) {
    context_name = name;
}