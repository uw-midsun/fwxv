#pragma once

#include "can.h"

typedef struct {
    // Initialize start_frame as 0xAA
    uint8_t start_frame;
    uint16_t id;
    size_t dlc;
    
    union {
    uint64_t data_u64;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
    } data;

    // Initialize end_of_frame as 0xBB
    uint8_t end_of_frame;

} Datagram;

Datagram decode_can_message(Datagram datagram, CanMessage msg);
