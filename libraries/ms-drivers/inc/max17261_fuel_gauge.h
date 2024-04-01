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

  uint16_t i_thresh_max;
  uint16_t i_thresh_min;
  uint16_t temp_thresh_max;

  float r_sense_mohms;  // Rsense in micro ohms
} Max17261Settings;

typedef struct {
  Max17261Settings *settings;
} Max17261Storage;

/* @brief Gets the current state of charge given by the max17261 in percentage
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - state of charge in percentage will be returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_state_of_charge(Max17261Storage *storage, uint16_t *soc_pct);

/* @brief Gets the current remaining capactity in micro amp hours
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - remaining capactity in micro amp hours returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_remaining_capacity(Max17261Storage *storage, uint32_t *rem_cap_uAhr);

/* @brief Gets the full charge capacity of the battery in micro amp hours
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - full charge capacitry in micro amp hours returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_full_capacity(Max17261Storage *storage, uint16_t *full_cap_uAhr);

/* @brief Gets the time to empty in milliseconds
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - time to empty in milliseconds returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_time_to_empty(Max17261Storage *storage, uint16_t *tte_ms);

/* @brief Gets the time to full in milliseconds
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - time to full in milliseconds returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_time_to_full(Max17261Storage *storage, uint16_t *ttf_ms);

/* @brief Gets a current reading in amps
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - current in amps returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_current(Max17261Storage *storage, int *current_a);

/* @brief Gets a single cell's voltage in mV
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - voltage in mV returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_voltage(Max17261Storage *storage, uint16_t *vcell_mv);

/* @brief Gets a temperature reading in celcius
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - temperature in celcius returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_temp(Max17261Storage *storage, uint16_t *temp_c);

/* @brief Gets the time to full in milliseconds
 * @param storage - a pointer to an uninitialized Max17261Storage struct
 * @param settings - populated settings struct
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_init(Max17261Storage *storage, Max17261Settings *settings);
