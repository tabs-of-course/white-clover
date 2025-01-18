#include "receiver.h"
#include <iostream>
#include <chrono>
#include <thread>

receiver::receiver(std::shared_ptr<message_channel> channel, std::atomic<bool>& running)
    : channel(channel), running(running) {}

std::optional<message> receiver::receive_message() {
    std::unique_lock<std::mutex> lock(channel->mutex);
    
    // Wait for a message or shutdown
    channel->cv.wait(lock, [this]() {
        return !channel->messages.empty() || !running;
    });
    
    if (channel->messages.empty()) {
        return std::nullopt;
    }
    
    message msg = std::move(channel->messages.front());
    channel->messages.pop();
    lock.unlock();
    channel->cv.notify_one();
    return msg;
}

std::vector<message> receiver::receive_batch(size_t max_messages) {
    std::vector<message> batch;
    std::unique_lock<std::mutex> lock(channel->mutex);
    
    // Wait until there are messages or we're no longer running
    channel->cv.wait(lock, [this]() { 
        return !channel->messages.empty() || !running; 
    });
    
    if (!running && channel->messages.empty()) {
        return batch;
    }
    
    size_t batch_size = std::min(max_messages, channel->messages.size());
    batch.reserve(batch_size);
    
    for (size_t i = 0; i < batch_size; ++i) {
        batch.push_back(std::move(channel->messages.front()));
        channel->messages.pop();
    }
    
    return batch;
}

void receiver::operator()() {
    while (running || !channel->messages.empty()) {
        auto batch = receive_batch(BATCH_SIZE);
        for (const auto& msg : batch) {
            std::cout << "Received command: " << msg.m_command 
                     << " msg_id: " << msg.m_msg_id 
                     << " message: " << msg.m_msg << std::endl;
        }
    }
}