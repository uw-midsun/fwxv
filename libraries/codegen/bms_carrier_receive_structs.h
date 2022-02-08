#pragma once

#include <stdint.h>

typedef struct {
    uint16_t set_relay_state_relay_mask;
    uint16_t set_relay_state_relay_state;
    uint8_t signal1;
    uint8_t signal2;
    uint16_t signal3;
    uint8_t signal4;
    uint32_t signal5;
    uint8_t signal1;
    uint8_t signal1;
    uint16_t signal1;
    uint8_t signal1;
    uint8_t signal1;
    uint64_t signal1;
    uint8_t signal1;
    uint32_t signal1;
    uint8_t signal1;
    uint16_t signal1;
} bms_carrier_receive_struct;

// transmit_struct
// typedef struct {
//     uint16_t set_relay_state_relay_mask;
//     uint16_t set_relay_state_relay_state;
//     uint16_t power_on_main_sequence_sequence;
// }
