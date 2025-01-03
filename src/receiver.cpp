#include "receiver.h"
#include <iostream>
#include <chrono>
#include <thread>

receiver::receiver(std::shared_ptr<message_channel> channel, std::atomic<bool>& running)
    : channel(channel), running(running) {}

std::optional<message> receiver::receive_message() {
    std::unique_lock<std::mutex> lock(channel->mutex);
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
    
    size_t batch_size = std::min(max_messages, channel->messages.size());
    batch.reserve(batch_size);
    
    for (size_t i = 0; i < batch_size; ++i) {
        batch.push_back(std::move(channel->messages.front()));
        channel->messages.pop();
    }
    
    lock.unlock();
    channel->cv.notify_one();
    return batch;
}

void receiver::operator()() {
    size_t received_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (running || !channel->messages.empty()) {
        auto batch = receive_batch(BATCH_SIZE);
        for (const auto& msg : batch) {
            // Updated to use new member names
            std::cout << "Received command: " << msg.m_command 
                     << " msg_id: " << msg.m_msg_id 
                     << " message: " << msg.m_msg << std::endl;
        }
        received_count += batch.size();
        
        if (batch.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Receiver completed. Messages received: " << received_count 
              << " Time taken: " << duration.count() << "ms"
              << " Throughput: " << (received_count * 1000.0 / duration.count()) 
              << " messages/second\n";
}