#pragma once

// Stores current readings from the ADS1259 in a ring buffer.
// Requires interrupts and soft timers to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "i2c.h"
#include "max17261_fuel_gauge.h"
#include "pwm.h"
#include "spi.h"
#include "status.h"
#include "tasks.h"

#define MAX17261_I2C_PORT (I2C_PORT_2)
#define MAX17261_I2C_ADDR (0x36)

// TODO (Adel C): Change these values to their actual values
#define CURRENT_SENSE_R_SENSE_MILLI_OHMS (0.5)
#define MAIN_PACK_DESIGN_CAPACITY \
  (1.0f / CURRENT_SENSE_R_SENSE_MILLI_OHMS)      // LSB = 5.0 (micro Volt Hours / R Sense)
#define MAIN_PACK_EMPTY_VOLTAGE (1.0f / 78.125)  // Only a 9-bit field, LSB = 78.125 (micro Volts)
#define CHARGE_TERMINATION_CURRENT (1.0f / (1.5625f / CURRENT_SENSE_R_SENSE_MILLI_OHMS))

// Thresholds for ALRT Pin
#define CURRENT_SENSE_MAX_CURRENT_A (58.2f)
#define CURRENT_SENSE_MIN_CURRENT_A (27.0f)  // Actually -27
#define CURRENT_SENSE_MAX_TEMP (60U)
#define CURRENT_SENSE_MAX_VOLTAGE (150U)
#define ALRT_PIN_V_RES_MICRO_V (400)

// Enum for GPIO IT alerts (just the one pin)
typedef enum { CURRENT_SENSE_RUN_CYCLE = 0, ALRT_GPIO_IT } CurrentSenseNotification;

typedef struct CurrentStorage {
  uint16_t soc;
  uint16_t current;
  uint16_t voltage;
  uint16_t temperature;
  int16_t average;
  uint32_t fuel_guage_cycle_ms;  // Time in ms between conversions (soft timer kicks)
} CurrentStorage;

StatusCode current_sense_fault_check();

StatusCode current_sense_run();

bool current_sense_is_charging();

StatusCode current_sense_init(CurrentStorage *storage, I2CSettings *i2c_settings,
                              uint32_t fuel_guage_cycle_ms);
