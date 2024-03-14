#pragma once

#include <stdint.h>

#include "current_sense.h"
#include "fault_bps.h"
#include "i2c.h"
#include "ltc_afe.h"
#include "status.h"

#define BMS_PERIPH_I2C_PORT I2C_PORT_2
#define BMS_PERIPH_I2C_SDA_PIN \
  { GPIO_PORT_B, 11 }
#define BMS_PERIPH_I2C_SCL_PIN \
  { GPIO_PORT_B, 10 }

#define MAX_CURRENT 27
#define MAX_AMBIENT_TEMP 75
#define MAX_CELL_TEMP 90

typedef struct BmsStorage {
  CurrentStorage current_storage;
  LtcAfeStorage ltc_afe_storage;
  BpsStorage bps_storage;
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
