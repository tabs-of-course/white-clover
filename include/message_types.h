#pragma once
#include <string>
#include <cstdint>

struct message {
    uint32_t m_command;
    uint32_t m_msg_id;
    std::string m_msg;
    std::string target_process_id;  
    int target_instance;           

    message() = default;
    message(uint32_t cmd, uint32_t id, std::string msg, 
            std::string target_id = "", int target_inst = -1)
        : m_command(cmd), m_msg_id(id), m_msg(std::move(msg))
        , target_process_id(std::move(target_id))
        , target_instance(target_inst) {}
};