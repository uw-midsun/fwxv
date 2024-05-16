#pragma once

#include <stdint.h>

#include "aux_sense.h"
#include "fault_bps.h"
#include "i2c.h"
#include "ltc_afe.h"
#include "max17261_fuel_gauge.h"
#include "status.h"

#define BMS_PERIPH_I2C_PORT I2C_PORT_2
#define BMS_PERIPH_I2C_SDA_PIN \
  { .port = GPIO_PORT_B, .pin = 11 }
#define BMS_PERIPH_I2C_SCL_PIN \
  { .port = GPIO_PORT_B, .pin = 10 }

typedef struct CurrentStorage {
  uint16_t soc;
  int16_t current;
  uint16_t voltage;
  uint16_t temperature;
  uint32_t fuel_guage_cycle_ms;  // Time in ms between conversions (soft timer kicks)
  uint32_t remaining_capacity;
  uint32_t full;
} CurrentStorage;

typedef struct BmsStorage {
  AuxStorage aux_storage;
  CurrentStorage current_storage;
  LtcAfeStorage ltc_afe_storage;
  BpsStorage bps_storage;
  Max17261Settings fuel_guage_settings;
  Max17261Storage fuel_guage_storage;
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
  BMS_FAULT_KILLSWITCH,
  BMS_FAULT_RELAY_CLOSE_FAILED,
} BmsFault;
