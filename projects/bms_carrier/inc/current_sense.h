#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bms.h"
#include "i2c.h"
#include "max17261_fuel_gauge.h"
#include "pwm.h"
#include "relays.h"
#include "spi.h"
#include "status.h"
#include "tasks.h"

#define MAX17261_I2C_PORT (I2C_PORT_2)
#define MAX17261_I2C_ADDR (0x36)

#define SENSE_RESISTOR_MOHM (0.5)

#define CELL_EMPTY_VOLTAGE_MV 2500  // LG M50T datasheet

#define CHARGE_TERMINATION_CURRENT_MA (400)  // 50 mA * 8 (one module)

// Thresholds for ALRT Pin
#define CURRENT_SENSE_MAX_CURRENT_A (-58.2f)
#define CURRENT_SENSE_MIN_CURRENT_A (27.0f)  // Actually 27
#define CURRENT_SENSE_MAX_TEMP_C (60U)
#define CURRENT_SENSE_MAX_VOLTAGE_V (152300)
#define ALRT_PIN_V_RES_MICRO_V (400)

#define MAX_SOLAR_CURRENT_A (25.0f)

#define CELL_X_R1_KOHMS 1780
#define CELL_X_R2_KOHMS 20

#define CURRENT_SENSE_STORE_FLASH NUM_FLASH_PAGES - 1

// Enum for GPIO IT alerts (just the one pin)
typedef enum { CURRENT_SENSE_RUN_CYCLE = 0, ALRT_GPIO_IT } CurrentSenseNotification;

StatusCode current_sense_fault_check();

StatusCode current_sense_run();

bool current_sense_is_charging();

StatusCode current_sense_init(BmsStorage *bms_storage, I2CSettings *i2c_settings,
                              uint32_t fuel_guage_cycle_ms);
