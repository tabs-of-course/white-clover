#include "receiver.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

receiver::receiver(std::shared_ptr<message_channel> channel, std::atomic<bool>& running)
    : channel(channel), running(running) {}

void receiver::operator()() {
    size_t received_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (running || !channel->messages.empty()) {
        std::vector<std::string> batch;
        batch.reserve(BATCH_SIZE);

        {
            std::unique_lock<std::mutex> lock(channel->mutex);
            channel->cv.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return !channel->messages.empty() || !running;
            });

            size_t batch_size = std::min(BATCH_SIZE, channel->messages.size());
            for (size_t i = 0; i < batch_size; ++i) {
                batch.push_back(std::move(channel->messages.front()));
                channel->messages.pop();
                received_count++;
            }
        }
        
        if (!batch.empty()) {
            channel->cv.notify_one();  // Notify sender that space is available
            
            // Process the batch (in this example, we're just counting)
            for (const auto& msg : batch) {
                // Process message here if needed
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Receiver completed. Messages received: " << received_count 
              << " Time taken: " << duration.count() << "ms"
              << " Throughput: " << (received_count * 1000.0 / duration.count()) 
              << " messages/second\n";
}
