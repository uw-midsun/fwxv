#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "global.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

typedef enum {
  CAN_HW_BITRATE_125KBPS,
  CAN_HW_BITRATE_250KBPS,
  CAN_HW_BITRATE_500KBPS,
  CAN_HW_BITRATE_1000KBPS,
  NUM_CAN_HW_BITRATES
} Boot_CanBitrate;

typedef enum {
  CAN_CONTINUOUS = 0,
  CAN_ONE_SHOT_MODE,
  NUM_CAN_MODES
} Boot_CanMode;

typedef struct Boot_CanSettings {
  uint16_t device_id;
  Boot_CanBitrate bitrate;
  bool loopback;
  Boot_CanMode mode;
} Boot_CanSettings;

typedef struct Boot_CanMessage {
  uint32_t id;
  uint8_t extended;
  size_t dlc;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
} Boot_CanMessage;

BootloaderError boot_can_init(const Boot_CanSettings *settings);

BootloaderError boot_can_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len);

BootloaderError boot_can_receive(Boot_CanMessage *msg);
