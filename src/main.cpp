#include "thread_manager.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Starting bidirectional message passing test...\n";
    
    thread_manager manager;
    manager.start_threads();
    
    // Print metrics periodically
    for (int i = 0; i < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        manager.print_metrics();
    }
    
    return 0;
}