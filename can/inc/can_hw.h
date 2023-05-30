#pragma once
// CAN HW Interface
// Requires GPIO and interrupts to be initialized.
//
// Used to initiate CAN TX and RX directly through the MCU, without any
// preprocessing or queues. Note that none of our systems currently support more
// than one CAN interface natively.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gpio.h"
#include "status.h"
#include "can_queue.h"

#ifdef CAN_HW_DEV_USE_CAN0
#define CAN_HW_DEV_INTERFACE "can0"
#else
#define CAN_HW_DEV_INTERFACE "vcan0"
#endif

typedef enum {
  CAN_HW_BUS_STATUS_OK = 0,
  CAN_HW_BUS_STATUS_ERROR,
  CAN_HW_BUS_STATUS_OFF
} CanHwBusStatus;

typedef enum {
  CAN_HW_BITRATE_125KBPS,
  CAN_HW_BITRATE_250KBPS,
  CAN_HW_BITRATE_500KBPS,
  CAN_HW_BITRATE_1000KBPS,
  NUM_CAN_HW_BITRATES
} CanHwBitrate;

typedef enum {
  CAN_CONTINUOUS = 0,
  CAN_ONE_SHOT_MODE,
  NUM_CAN_MODES
} CanMode;

typedef struct CanSettings {
  uint16_t device_id;
  CanHwBitrate bitrate;
  GpioAddress tx;
  GpioAddress rx;
  bool loopback;
  CanMode mode;
} CanSettings;

// Initializes CAN using the specified settings.
StatusCode can_hw_init(const CanQueue* rx_queue, const CanSettings *settings);

StatusCode can_hw_add_filter_in(uint32_t mask, uint32_t filter, bool extended);

CanHwBusStatus can_hw_bus_status(void);

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len);

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len);
