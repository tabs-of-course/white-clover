#pragma once
#include <string>
#include <cstdint>

struct message {
    uint32_t m_command;
    uint32_t m_msg_id;
    std::string m_msg;

    message() = default;
    message(uint32_t cmd, uint32_t id, std::string msg)
        : m_command(cmd), m_msg_id(id), m_msg(std::move(msg)) {}
};