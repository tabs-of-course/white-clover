#include "sender.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

sender::sender(std::shared_ptr<message_channel> channel, std::atomic<bool>& running)
    : channel(channel), running(running) {}

bool sender::send_message(const message& msg) {
    std::unique_lock<std::mutex> lock(channel->mutex);
    if (channel->messages.size() >= channel->MAX_QUEUE_SIZE) {
        return false;
    }
    channel->messages.push(msg);
    lock.unlock();
    channel->cv.notify_one();
    return true;
}

bool sender::send_batch(const std::vector<message>& messages) {
    std::unique_lock<std::mutex> lock(channel->mutex);
    if (channel->messages.size() + messages.size() > channel->MAX_QUEUE_SIZE) {
        return false;
    }
    for (const auto& msg : messages) {
        channel->messages.push(msg);
    }
    lock.unlock();
    channel->cv.notify_one();
    return true;
}

void sender::operator()() {
    size_t message_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (running && message_count < 1000000) {
        std::vector<message> batch;
        batch.reserve(BATCH_SIZE);

        for (size_t i = 0; i < BATCH_SIZE && running && message_count < 1000000; ++i) {
            // Updated to use new member names
            batch.emplace_back(1, message_count, "Message " + std::to_string(message_count));
            message_count++;
        }

        while (!send_batch(batch) && running) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Sender completed. Messages sent: " << message_count 
              << " Time taken: " << duration.count() << "ms"
              << " Throughput: " << (message_count * 1000.0 / duration.count()) 
              << " messages/second\n";
}