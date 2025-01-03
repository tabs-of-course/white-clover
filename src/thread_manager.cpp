#include "thread_manager.h"
#include "thread_context.h"
#include <iostream>

thread_manager::thread_manager()
    : channel_1_to_2(std::make_shared<message_channel>())
    , channel_2_to_1(std::make_shared<message_channel>()) {
    
    // Create contexts with appropriate channels
    context1 = std::make_unique<thread_context>(channel_1_to_2, channel_2_to_1, running);
    context2 = std::make_unique<thread_context>(channel_2_to_1, channel_1_to_2, running);

    // Set names for better logging
    context1->set_name("Context1");
    context2->set_name("Context2");
}

thread_manager::~thread_manager() {
    stop_threads();
}

void thread_manager::start_threads() {
    std::cout << "Starting threads..." << std::endl;
    context1->start();
    context2->start();
}

void thread_manager::stop_threads() {
    std::cout << "Stopping threads..." << std::endl;
    running = false;
    
    if (context1) context1->stop();
    if (context2) context2->stop();

    channel_1_to_2->cv.notify_all();
    channel_2_to_1->cv.notify_all();
    
    // Print final metrics
    print_metrics();
}

void thread_manager::print_metrics() const {
    std::cout << "\n=== System Metrics ===" << std::endl;
    context1->print_metrics();
    context2->print_metrics();
    std::cout << "==================\n" << std::endl;
}