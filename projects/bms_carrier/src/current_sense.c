#include "current_sense.h"

#include <inttypes.h>
#include <math.h>
#include <string.h>

#include "bms.h"
#include "bms_carrier_setters.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"
#include "tasks.h"

static Max17261Storage *s_fuel_guage_storage;
static Max17261Settings *s_fuel_gauge_settings;
static BmsStorage *s_storage;
static Max27261Params s_fuel_params;

StatusCode prv_fuel_gauge_read() {
  StatusCode status = STATUS_CODE_OK;

  status |= max17261_current(s_fuel_guage_storage, &s_storage->pack_current);
  non_blocking_delay_ms(5);
  status |= max17261_voltage(s_fuel_guage_storage, &s_storage->pack_voltage);
  non_blocking_delay_ms(5);
  status |= max17261_temp(s_fuel_guage_storage, &s_storage->temperature);
  non_blocking_delay_ms(5);

  // Measured voltage corresponds to one cell. Multiply it by the number of cells in series
  s_storage->pack_voltage = s_storage->pack_voltage * s_storage->config.series_count;

  LOG_DEBUG("CURRENT: %" PRIu32 "\n", s_storage->pack_current);
  LOG_DEBUG("VOLTAGE: %" PRIu32 "\n", s_storage->pack_voltage);
  LOG_DEBUG("TEMP: %d\n", s_storage->temperature);

  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Status error: %d\n", status);
    fault_bps_set(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    return status;
  }

  // Set Battery VT message signals
  set_battery_vt_current((uint16_t)abs(s_storage->pack_current));
  set_battery_vt_voltage((uint16_t)(s_storage->pack_voltage / 10));
  set_battery_vt_temperature(s_storage->temperature);

  if (s_storage->pack_current >= MAX_SOLAR_CURRENT_A * 1000) {
    bms_open_solar();
  }
  if (s_storage->pack_current <= CURRENT_SENSE_MAX_CURRENT_A * 1000) {
    fault_bps_set(BMS_FAULT_OVERCURRENT);
    return STATUS_CODE_INTERNAL_ERROR;
  } else if (s_storage->pack_current >= CURRENT_SENSE_MIN_CURRENT_A * 1000) {
    fault_bps_set(BMS_FAULT_OVERCURRENT);
    return STATUS_CODE_INTERNAL_ERROR;
  } else if (s_storage->pack_voltage >= CURRENT_SENSE_MAX_VOLTAGE_V) {
    fault_bps_set(BMS_FAULT_OVERVOLTAGE);
    return STATUS_CODE_INTERNAL_ERROR;
  } else if (s_storage->temperature >= CURRENT_SENSE_MAX_TEMP_C) {
    fault_bps_set(BMS_FAULT_OVERTEMP_AMBIENT);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return status;
}

TASK(current_sense, TASK_STACK_512) {
  uint32_t notification = 0;
  while (true) {
    notify_wait(&notification, BLOCK_INDEFINITELY);
    LOG_DEBUG("Running Current Sense Cycle!\n");

    // Handle alert from fuel gauge
    // if (notification & (1 << ALRT_GPIO_IT)) {
    //   LOG_DEBUG("ALERT_PIN triggered\n");
    //   fault_bps_set(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    // }

    prv_fuel_gauge_read();
    send_task_end();
  }
}

StatusCode current_sense_run() {
  StatusCode ret = notify(current_sense, CURRENT_SENSE_RUN_CYCLE);
  if (ret != STATUS_CODE_OK) {
    fault_bps_set(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode current_sense_init(BmsStorage *bms_storage, I2CSettings *i2c_settings,
                              uint32_t fuel_guage_cycle_ms) {
  StatusCode status = STATUS_CODE_OK;
  s_storage = bms_storage;
  s_fuel_gauge_settings = &s_storage->fuel_guage_settings;
  s_fuel_guage_storage = &s_storage->fuel_guage_storage;
  i2c_init(MAX17261_I2C_PORT, i2c_settings);

  /* Initialize current sense ALRT pin */
  GpioAddress alrt_pin = { .port = GPIO_PORT_A, .pin = 7 };

  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_RISING,
  };
  gpio_it_register_interrupt(&alrt_pin, &it_settings, ALRT_GPIO_IT, current_sense);

  /* Initialize fuel gauge IC */
  s_fuel_gauge_settings->i2c_port = MAX17261_I2C_PORT;
  s_fuel_gauge_settings->i2c_address = MAX17261_I2C_ADDR;

  s_fuel_gauge_settings->charge_term_current_ma = CHARGE_TERMINATION_CURRENT_MA;
  s_fuel_gauge_settings->pack_design_cap_mah = 38800;
  s_fuel_gauge_settings->cell_empty_voltage_v = CELL_EMPTY_VOLTAGE_MV;

  s_fuel_gauge_settings->sense_resistor_mohms = SENSE_RESISTOR_MOHM;

  status = max17261_init(s_fuel_guage_storage, s_fuel_gauge_settings, &s_fuel_params);
  tasks_init_task(current_sense, TASK_PRIORITY(2), NULL);
  return status;
}
