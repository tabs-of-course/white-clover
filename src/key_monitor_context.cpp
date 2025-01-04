#include "key_monitor_context.h"
#include <iostream>
#include <sstream>

key_monitor_context::key_monitor_context(std::shared_ptr<message_channel> outbound_channel,
                                       std::shared_ptr<message_channel> inbound_channel,
                                       std::atomic<bool>& running)
    : outbound_channel(outbound_channel)
    , inbound_channel(inbound_channel)
    , running(running)
    , msg_sender(outbound_channel, running)
    , msg_receiver(inbound_channel, running) {
    std::cout << "Key monitor context created" << std::endl;
}

void key_monitor_context::operator()() {
    std::cout << "Key monitor thread started" << std::endl;
    uint32_t msg_id = 0;

    // Array to store key states
    bool previous_state[256] = {false};

    while (running) {
        // Check each key
        for (int vk = 0; vk < 256; vk++) {
            bool current_state = (GetAsyncKeyState(vk) & 0x8000) != 0;

            // Detect key press (transition from not pressed to pressed)
            if (current_state && !previous_state[vk]) {
                std::string key_name = get_key_name(vk);
                if (!key_name.empty()) {
                    std::cout << "Key detected: " << key_name << std::endl;
                    
                    // Create and send message about key press
                    message key_msg(2, msg_id++, "Key pressed: " + key_name);
                    if (msg_sender.send_message(key_msg)) {
                        messages_sent++;
                        keys_processed++;
                    }
                }
            }
            previous_state[vk] = current_state;
        }

        // Process any incoming messages
        auto received = msg_receiver.receive_batch(10);
        for (const auto& msg : received) {
            process_message(msg);
        }

        // Very short sleep to prevent excessive CPU usage
        Sleep(1);  // 1ms sleep instead of 10ms
    }
}

void key_monitor_context::process_message(const message& msg) {
    std::cout << context_name << " received message - Command: " << msg.m_command 
              << " ID: " << msg.m_msg_id 
              << " Content: " << msg.m_msg << std::endl;
    messages_processed++;
}

void key_monitor_context::start() {
    worker_thread = std::thread(&key_monitor_context::operator(), this);
}

void key_monitor_context::stop() {
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void key_monitor_context::print_metrics() const {
    std::cout << context_name << " Metrics:"
              << " Keys Processed: " << keys_processed
              << " Messages Processed: " << messages_processed
              << " Messages Sent: " << messages_sent
              << " Inbound Queue: " << inbound_channel->messages.size()
              << " Outbound Queue: " << outbound_channel->messages.size()
              << std::endl;
}

void key_monitor_context::set_name(const std::string& name) {
    context_name = name;
}

std::string key_monitor_context::get_key_name(DWORD vk_code) {
    // Handle special keys
    switch (vk_code) {
        case VK_RETURN: return "Enter";
        case VK_SPACE: return "Space";
        case VK_BACK: return "Backspace";
        case VK_TAB: return "Tab";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_ESCAPE: return "Esc";
        case VK_LEFT: return "Left";
        case VK_UP: return "Up";
        case VK_RIGHT: return "Right";
        case VK_DOWN: return "Down";
        default:
            // For standard keys, get the key name from Windows
            UINT scan_code = MapVirtualKey(vk_code, MAPVK_VK_TO_VSC);
            if (scan_code == 0) return "";

            char key_name[32];
            if (GetKeyNameTextA(scan_code << 16, key_name, sizeof(key_name)) > 0) {
                return std::string(key_name);
            }
            return "";
    }
}