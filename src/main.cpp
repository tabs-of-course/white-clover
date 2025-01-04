#include "thread_manager.h"
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Starting bidirectional message passing test with key monitoring...\n";
    
    thread_manager manager;
    manager.start_threads();
    
    // Keep the program running and print metrics less frequently
    while (true) {
        // manager.print_metrics();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Optional: check for exit condition
        if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
            std::cout << "ESC pressed, exiting..." << std::endl;
            break;
        }
    }
    
    return 0;
}