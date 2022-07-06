#pragma once

#include <vector>

#include "events_code.h"

struct EventPacket {
    uint8_t eventCode;
    uint8_t length;
    std::vector<uint8_t> params;
    uint32_t time;

    EventPacket() : eventCode(0), length(0), time(0){};

    bool isFromCommand(OpCodeCommand command)
    {
        return params.size() >= 3 && params[0] == 0x01 && params[1] == ((uint16_t)command & 0x00FF) &&
               params[2] == ((uint16_t)command >> 8);
    }
};

struct AsyncDataPacket {
    uint16_t handle;
    uint8_t flags;
    uint16_t length;
    std::vector<uint8_t> data;
    uint32_t time;

    AsyncDataPacket() : handle(0), flags(0), length(0), time(0){};
};