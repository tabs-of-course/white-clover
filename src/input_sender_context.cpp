#include "input_sender_context.h"
#include <iostream>
#include <sstream>

input_sender_context::input_sender_context(std::shared_ptr<message_channel> outbound_channel,
                                         std::shared_ptr<message_channel> inbound_channel,
                                         std::atomic<bool>& running,
                                         HWND target_window,
                                         const std::string& process_id,
                                         int instance_num)
    : outbound_channel(outbound_channel)
    , inbound_channel(inbound_channel)
    , running(running)
    , msg_sender(outbound_channel, running)
    , msg_receiver(inbound_channel, running)
    , target_hwnd(target_window)
    , process_id(process_id)
    , instance_number(instance_num) {
    std::cout << "Input sender context created for window handle: 0x" 
              << std::hex << (uintptr_t)target_window << std::dec
              << " (Process: " << process_id << ", Instance: " << instance_num << ")" 
              << std::endl;
}

void input_sender_context::operator()() {
    std::cout << context_name << " thread started" << std::endl;

    while (running) {
        auto received = msg_receiver.receive_batch(10);
        for (const auto& msg : received) {
            // Check if this message is for us
            if ((msg.target_process_id == process_id) && 
                (msg.target_instance == -1 || msg.target_instance == instance_number)) {
                process_message(msg);
            }
        }
        Sleep(1);
    }
}

void input_sender_context::process_message(const message& msg) {
    // Get window title for logging
    char window_title[256];
    GetWindowTextA(target_hwnd, window_title, sizeof(window_title));
    
    std::cout << "\nProcessing key in " << context_name << ":\n"
              << "  Key: " << msg.m_msg << "\n"
              << "  Target Window: 0x" << std::hex << (uintptr_t)target_hwnd << std::dec << "\n"
              << "  Window Title: " << window_title << "\n";

    messages_processed++;

    // Check if it's a key press message
    if (msg.m_command == 2) {  // Command 2 is for key press events
        // Extract key name from message (format: "Key pressed: X")
        size_t prefix_len = strlen("Key pressed: ");
        if (msg.m_msg.length() > prefix_len) {
            std::string key_name = msg.m_msg.substr(prefix_len);
            std::cout << "  Sending key '" << key_name << "' to window\n";
            send_key_to_window(key_name);
        }
    }
}

void input_sender_context::send_key_to_window(const std::string& key_name) {
    WORD vk_code = get_virtual_key_code(key_name);
    if (vk_code != 0) {
        std::cout << "Sending key: " << key_name 
                  << " (VK: 0x" << std::hex << vk_code << std::dec << ")"
                  << " to window: 0x" << std::hex << (uintptr_t)target_hwnd << std::dec << std::endl;

        if (!IsWindow(target_hwnd)) {
            std::cout << "ERROR: Target window is not valid!" << std::endl;
            return;
        }

        simulate_key_press(vk_code);
        inputs_sent++;
    }
}

void input_sender_context::simulate_key_press(WORD vk_code) {
    UINT scan_code;
    if ((vk_code >= 'A' && vk_code <= 'Z') || (vk_code >= '0' && vk_code <= '9')) {
        scan_code = MapVirtualKeyW(vk_code, MAPVK_VK_TO_VSC_EX);
    } else {
        scan_code = MapVirtualKeyW(vk_code, MAPVK_VK_TO_VSC);
    }
    
    // Construct detailed lParam
    LPARAM lParam = 1                     // Repeat count (bits 0-15)
                    | (scan_code << 16)   // Scan code (bits 16-23)
                    | (0 << 24)           // Extended key flag
                    | (0 << 25)           // Don't care (bit 25)
                    | (0 << 26)           // Alt state (bit 26)
                    | (0 << 27)           // Don't care (bit 27)
                    | (0 << 28)           // Previous state (bit 28)
                    | (0 << 29)           // Transition state (bit 29)
                    | (0 << 30)           // Don't care (bit 30)
                    | (0 << 31);          // Don't care (bit 31)

    std::cout << "Sending key 0x" << std::hex << vk_code 
              << " with scan code 0x" << scan_code 
              << " lParam: 0x" << lParam << std::dec << std::endl;

    // Send key down
    SendMessageW(target_hwnd, WM_KEYDOWN, vk_code, lParam);
    
    // Set transition and previous state flags for key up
    lParam |= (1 << 30) | (1 << 29);
    SendMessageW(target_hwnd, WM_KEYUP, vk_code, lParam);
}

WORD input_sender_context::get_virtual_key_code(const std::string& key_name) {
    // Handle special keys
    static const std::unordered_map<std::string, WORD> special_keys = {
        {"Enter", VK_RETURN},
        {"Space", VK_SPACE},
        {"Backspace", VK_BACK},
        {"Tab", VK_TAB},
        {"Shift", VK_SHIFT},
        {"Ctrl", VK_CONTROL},
        {"Alt", VK_MENU},
        {"Esc", VK_ESCAPE},
        {"Left", VK_LEFT},
        {"Up", VK_UP},
        {"Right", VK_RIGHT},
        {"Down", VK_DOWN}
    };

    std::cout << "Converting key name: " << key_name << std::endl;

    // Check special keys first
    auto it = special_keys.find(key_name);
    if (it != special_keys.end()) {
        std::cout << "Found special key, VK: 0x" << std::hex << it->second << std::dec << std::endl;
        return it->second;
    }

    // For single characters (letters or numbers)
    if (key_name.length() == 1) {
        char c = key_name[0];
        if (isalpha(c)) {
            // Handle letters (A-Z)
            WORD vk = static_cast<WORD>(toupper(c));
            std::cout << "Converted letter to VK: 0x" << std::hex << vk << std::dec << std::endl;
            return vk;
        }
        else if (isdigit(c)) {
            // Handle numbers (0-9)
            WORD vk = static_cast<WORD>(c);
            std::cout << "Converted number to VK: 0x" << std::hex << vk << std::dec << std::endl;
            return vk;
        }
    }

    std::cout << "No conversion found for key: " << key_name << std::endl;
    return 0;  // Unknown key
}

void input_sender_context::start() {
    worker_thread = std::thread(&input_sender_context::operator(), this);
}

void input_sender_context::stop() {
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void input_sender_context::print_metrics() const {
    std::cout << context_name << " Metrics:"
              << " Messages Processed: " << messages_processed
              << " Messages Sent: " << messages_sent
              << " Inputs Sent: " << inputs_sent
              << " Inbound Queue: " << inbound_channel->messages.size()
              << " Outbound Queue: " << outbound_channel->messages.size()
              << std::endl;
}

void input_sender_context::set_name(const std::string& name) {
    context_name = name;
}