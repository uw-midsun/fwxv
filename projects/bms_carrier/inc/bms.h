#pragma once

#include <stdint.h>

#include "i2c.h"
#include "ltc_afe.h"
#include "max17261_fuel_gauge.h"
#include "status.h"

#define BMS_PERIPH_I2C_SDA_PIN \
  { .port = GPIO_PORT_B, .pin = 11 }
#define BMS_PERIPH_I2C_SCL_PIN \
  { .port = GPIO_PORT_B, .pin = 10 }

#define MAX17261_I2C_PORT (I2C_PORT_2)
#define MAX17261_I2C_ADDR (0x36)

#define CELL_CAPACITY_MAH 4850
#define NUM_SERIES_CELLS (4 * 9)
#define NUM_PARALLEL_CELLS (8)
#define PACK_CAPACITY_MAH (CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS)

typedef struct {
  uint8_t series_count;
  uint8_t parallel_count;
  uint16_t pack_capacity;
} BmsConfig;

typedef struct {
  int32_t pack_current;   // mA
  uint32_t pack_voltage;  // mV
  uint16_t temperature;
  uint16_t aux_batt_voltage;  // mV
  uint16_t fault_bitset;
  BmsConfig config;

  LtcAfeStorage ltc_afe_storage;
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
  BMS_FAULT_DISCONNECTED
} BmsFault;
