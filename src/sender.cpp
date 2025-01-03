#include "sender.h"
#include <iostream>
#include <chrono>
#include <vector>

sender::sender(std::shared_ptr<message_channel> channel, std::atomic<bool>& running)
    : channel(channel), running(running) {}

void sender::operator()() {
    size_t message_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (running && message_count < 1000000) {  // Send 1M messages
        std::vector<std::string> batch;
        batch.reserve(BATCH_SIZE);

        // Prepare batch
        for (size_t i = 0; i < BATCH_SIZE && running && message_count < 1000000; ++i) {
            batch.push_back("Message " + std::to_string(message_count++));
        }

        // Send batch
        {
            std::unique_lock<std::mutex> lock(channel->mutex);
            channel->cv.wait(lock, [this]() { 
                return channel->messages.size() < channel->MAX_QUEUE_SIZE || !running; 
            });

            if (!running) break;

            for (auto& msg : batch) {
                channel->messages.push(std::move(msg));
            }
        }
        channel->cv.notify_one();  // Notify receiver
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Sender completed. Messages sent: " << message_count 
              << " Time taken: " << duration.count() << "ms"
              << " Throughput: " << (message_count * 1000.0 / duration.count()) 
              << " messages/second\n";
}