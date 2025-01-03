#include "thread_manager.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Starting high-performance message passing benchmark...\n";
    
    thread_manager manager;
    manager.start_threads();
    
    // Let it run for a while
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}