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

  uint32_t pack_design_cap_mah;
  uint16_t cell_empty_voltage_v;
  uint16_t
      charge_term_current_ma;  // ref end-of-charge detection
                               // https://web.archive.org/web/20220121025712mp_/https://pdfserv.maximintegrated.com/en/an/user-guide-6597-max1726x-m5-ez-rev3-p4.pdf

  uint16_t i_thresh_max_a;
  int16_t i_thresh_min_a;
  uint16_t temp_thresh_max_c;

  float sense_resistor_mohms;
} Max17261Settings;

typedef struct {
  Max17261Settings *settings;
} Max17261Storage;

// Storage for parameters learned by fuel guage
// Must be stored in flash to keep up to date after power cycle
typedef struct Max27261Params {
  uint16_t rcomp0;
  uint16_t tempco;
  uint16_t fullcaprep;
  uint16_t cycles;
  uint16_t fullcapnom;
} Max27261Params;

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
StatusCode max17261_remaining_capacity(Max17261Storage *storage, uint32_t *rem_cap_mAh);

/* @brief Gets the full charge capacity of the battery in micro amp hours
 * @param storage - a pointer to an already initialized Max17261Storage struct
 * @param soc_pct - full charge capacitry in micro amp hours returned in this var
 * @return STATUS_CODE_OK on success
 */
StatusCode max17261_full_capacity(Max17261Storage *storage, uint32_t *full_cap_mAh);

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
StatusCode max17261_current(Max17261Storage *storage, int16_t *current_a);

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
StatusCode max17261_init(Max17261Storage *storage, Max17261Settings *settings,
                         Max27261Params *params);

StatusCode max17261_set_learned_params(Max17261Storage *storage, Max27261Params *params);
StatusCode max17261_get_learned_params(Max17261Storage *storage, Max27261Params *params);
