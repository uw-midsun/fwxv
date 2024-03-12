#pragma once

#include <stdint.h>

#include "cell_sense.h"
#include "current_sense.h"
#include "i2c.h"
#include "status.h"

#define BMS_PERIPH_I2C_PORT I2C_PORT_2
#define BMS_PERIPH_I2C_SDA_PIN \
  { GPIO_PORT_B, 11 }
#define BMS_PERIPH_I2C_SCL_PIN \
  { GPIO_PORT_B, 10 }
#define BMS_FAN_ALERT_PIN \
  { GPIO_PORT_A, 9 }

#define BMS_IO_EXPANDER_I2C_ADDR 0x40

#define BMS_FAN_CTRL_1_I2C_ADDR 0x5E
#define BMS_FAN_CTRL_2_I2C_ADDR 0x5F

#define MAX_VOLTAGE 42000
#define MIN_VOLTAGE 40000

// Fault thresholds
#define OVERVOLTAGE_THRESHOLD 42500
#define UNDERVOLTAGE_THRESHOLD 25000
#define AFE_BALANCING_UPPER_THRESHOLD 41500
#define AFE_BALANCING_LOWER_THRESHOLD 40000
#define MAX_CURRENT 27
#define MAX_AMBIENT_TEMP 75
#define MAX_CELL_TEMP 90

// Not dealing with debouncer here
typedef struct BmsStorage {
  // RelayStorage relay_storage;
  CurrentStorage current_storage;
  AfeReadings afe_readings;
  LtcAfeStorage ltc_afe_storage;
  CellSenseStorage cell_storage;
  // FanStorage fan_storage_1;
  // FanStorage fan_storage_2;
  // DebouncerStorage killswitch_storage;
  // BpsStorage bps_storage;
} BmsStorage;

typedef enum {
  BMS_FAULT_OVERVOLTAGE,
  BMS_FAULT_UNBALANCE,
  BMS_FAULT_OVERTEMP_AMBIENT,
  BMS_FAULT_COMMS_LOSS_AFE,
  BMS_FAULT_COMMS_LOSS_CURR_SENSE,
  BMS_FAULT_OVERTEMP_CELL,
  BMS_FAULT_OVERCURRENT,
  BMS_FAULT_UNDERVOLTAGE,
  BMS_FAULT_KILLSWITCH
} BmsFault;
