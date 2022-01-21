#pragma once

#include <stdbool.h>

// For setting the CAN device
typedef enum {
    SYSTEM_CAN_DEVICE_CENTRE_CONSOLE = 0,
    SYSTEM_CAN_DEVICE_BMS_CARRIER = 1,
    SYSTEM_CAN_DEVICE_PEDAL = 2,
    NUM_SYSTEM_CAN_DEVICES = 3
} SystemCanDevice;