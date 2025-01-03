#include "thread_manager.h"
#include <iostream>

thread_manager::thread_manager()
    : channel(std::make_shared<message_channel>()),
      msg_sender(channel, running),
      msg_receiver(channel, running) {}

thread_manager::~thread_manager() {
    running = false;
    channel->cv.notify_all();  // Wake up any waiting threads
    if (sender_thread.joinable()) sender_thread.join();
    if (receiver_thread.joinable()) receiver_thread.join();
}

void thread_manager::start_threads() {
    sender_thread = std::thread(std::ref(msg_sender));
    receiver_thread = std::thread(std::ref(msg_receiver));
}

void thread_manager::print_metrics() const {
    std::lock_guard<std::mutex> lock(channel->mutex);
    std::cout << "Current queue size: " << channel->messages.size() << "\n";
}