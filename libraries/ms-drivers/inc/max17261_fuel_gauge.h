#pragma once
#include <stdint.h>

#include "i2c.h"
#include "max17261_fuel_gauge_defs.h"

/* Notes:
 * 3 things are required for configuring the max17261:
 * - Design Capacity (in milliamp hours)
 * - Empty Voltage
 * - Charge Termination Current
 *
 * Some of the units of the registers (like capacity and current) depend on RSense,
 * it looks like max17261 automatically determines this value, it should be provided
 * to the driver for conversions from register values to actual units
 *
 * See data sheet P.15 for more info on how registers are formatted
 * Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/max17261.pdf
 */

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;

  uint16_t design_capacity;      // LSB = 5.0 (micro Volt Hours / R Sense)
  uint16_t empty_voltage;        // Only a 9-bit field, LSB = 78.125 (micro Volts)
  uint16_t charge_term_current;  // LSB = 1.5625 (micro Volts / R Sense)

  uint16_t r_sense_ohms;
} Max17261Settings;

typedef struct {
  Max17261Settings settings;
} Max17261Storage;

StatusCode max17261_get_reg(Max17261Storage *storage, Max17261Registers reg, uint16_t *value);
StatusCode max17261_set_reg(Max17261Storage *storage, Max17261Registers reg, uint16_t value);
StatusCode max17261_state_of_charge(Max17261Storage *storage, uint16_t *soc_pct);
StatusCode max17261_remaining_capacity(Max17261Storage *storage, uint32_t *rem_cap_uAhr);
StatusCode max17261_full_capacity(Max17261Storage *storage, uint16_t *full_cap_uAhr);
StatusCode max17261_time_to_empty(Max17261Storage *storage, uint16_t *tte_ms);
StatusCode max17261_time_to_full(Max17261Storage *storage, uint16_t *ttf_ms);
StatusCode max17261_init(Max17261Storage *storage, Max17261Settings settings);
