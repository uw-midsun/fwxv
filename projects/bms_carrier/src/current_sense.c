#include "current_sense.h"

#include <inttypes.h>
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

// Update stored learned params every 30 sec
#define UPDATE_FLASH_PARAMS_PERIOD_MS 10000

static Max17261Storage s_fuel_guage_storage;
static Max17261Settings s_fuel_gauge_settings;
static CurrentStorage *s_current_storage;
static bool s_is_charging;

static PersistStorage s_persist;
static Max27261Params s_fuel_params;
static TickType_t s_last_params_update;

// Periodically read and update the SoC of the car & update charging bool
StatusCode prv_fuel_gauge_read() {
  StatusCode status = STATUS_CODE_OK;

  status |= max17261_state_of_charge(&s_fuel_guage_storage, &s_current_storage->soc);
  status |= max17261_current(&s_fuel_guage_storage, &s_current_storage->current);
  status |= max17261_voltage(&s_fuel_guage_storage, &s_current_storage->voltage);
  status |= max17261_temp(&s_fuel_guage_storage, &s_current_storage->temperature);
  status |= max17261_remaining_capacity(&s_fuel_guage_storage, &s_current_storage->mah);
  status |= max17261_full_capacity(&s_fuel_guage_storage, &s_current_storage->full);

  LOG_DEBUG("SOC: %d\n", s_current_storage->soc);
  LOG_DEBUG("CURRENT: %d\n", s_current_storage->current);
  LOG_DEBUG("VOLTAGE: %d\n", s_current_storage->voltage);
  LOG_DEBUG("TEMP: %d\n", s_current_storage->temperature);
  LOG_DEBUG("mah: %" PRIu32 "\n", s_current_storage->mah);
  LOG_DEBUG("Full: %" PRIu32 "\n", s_current_storage->full);

  if (status != STATUS_CODE_OK) {
    // TODO (Adel): Handle a fuel gauge fault
    // Open Relays
    LOG_DEBUG("Status error: %d\n", status);
    fault_bps_set(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    return status;
  }

  // Set Battery VT message signals
  set_battery_vt_batt_perc(s_current_storage->soc);
  set_battery_vt_current((uint16_t)s_current_storage->current);
  set_battery_vt_voltage(s_current_storage->voltage);
  set_battery_vt_temperature(s_current_storage->temperature);

  // TODO (Aryan): Validate these checks
  // if (s_current_storage->current >= CURRENT_SENSE_MAX_CURRENT_A * 1000) {
  //   fault_bps_set(BMS_FAULT_OVERCURRENT);
  //   return STATUS_CODE_INTERNAL_ERROR;
  // } else if (s_current_storage->voltage >= CURRENT_SENSE_MAX_VOLTAGE) {
  //   fault_bps_set(BMS_FAULT_OVERVOLTAGE);
  //   return STATUS_CODE_INTERNAL_ERROR;
  // } else if (s_current_storage->temperature >= CURRENT_SENSE_MAX_TEMP) {
  //   fault_bps_set(BMS_FAULT_OVERTEMP_AMBIENT);
  //   return STATUS_CODE_INTERNAL_ERROR;
  // }

  // Commit params info periodically
  if (xTaskGetTickCount() > s_last_params_update + pdMS_TO_TICKS(UPDATE_FLASH_PARAMS_PERIOD_MS)) {
    LOG_DEBUG("Updating params\n");
    s_last_params_update = xTaskGetTickCount();
    max17261_get_learned_params(&s_fuel_guage_storage, &s_fuel_params);
    persist_commit(&s_persist);
  }
  return status;
}

TASK(current_sense, TASK_STACK_256) {
  while (true) {
    uint32_t notification = 0;
    notify_wait(&notification, BLOCK_INDEFINITELY);
    LOG_DEBUG("Running Current Sense Cycle!\n");

    // Handle alert from fuel gauge
    if (notification & (1 << ALRT_GPIO_IT)) {
      // fault_bps_set(BMS_FAULT_COMMS_LOSS_CURR_SENSE);
    } else if (notification & 1 << KILLSWITCH_IT) {
      fault_bps_set(BMS_FAULT_KILLSWITCH);
    }
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
  s_current_storage = &bms_storage->current_storage;
  s_fuel_gauge_settings = bms_storage->fuel_guage_settings;
  s_fuel_guage_storage = bms_storage->fuel_guage_storage;
  i2c_init(I2C_PORT_2, i2c_settings);

  GpioAddress alrt_pin = { .port = GPIO_PORT_A, .pin = 7 };
  GpioAddress kill_switch_mntr = { .port = GPIO_PORT_A, .pin = 15 };

  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_RISING,
  };
  gpio_init_pin(&kill_switch_mntr, GPIO_INPUT_FLOATING, GPIO_STATE_LOW);
  gpio_it_register_interrupt(&alrt_pin, &it_settings, ALRT_GPIO_IT, current_sense);
  it_settings.edge = INTERRUPT_EDGE_FALLING;
  gpio_it_register_interrupt(&kill_switch_mntr, &it_settings, KILLSWITCH_IT, current_sense);

  s_fuel_gauge_settings.i2c_port = I2C_PORT_2;
  s_fuel_gauge_settings.i2c_address = MAX17261_I2C_ADDR;

  s_fuel_gauge_settings.charge_term_current_ma = CHARGE_TERMINATION_CURRENT_MA;
  s_fuel_gauge_settings.pack_design_cap_mah = PACK_CAPACITY_MAH;
  s_fuel_gauge_settings.cell_empty_voltage_v = CELL_EMPTY_VOLTAGE_MV;

  // Expected MAX current / (uV / uOhmsSense) resolution
  // TODO impl alert
  // s_fuel_gauge_settings.i_thresh_max =
  //     ((CURRENT_SENSE_MAX_CURRENT_A) / (ALRT_PIN_V_RES_MICRO_V / CURRENT_SENSE_R_SENSE_MOHMS));
  // // Expected MIN current / (uV / uOhmsSense) resolution
  // s_fuel_gauge_settings.i_thresh_min =
  //     ((CURRENT_SENSE_MIN_CURRENT_A) / (ALRT_PIN_V_RES_MICRO_V / CURRENT_SENSE_R_SENSE_MOHMS));
  // // Interrupt threshold limits are stored in 2s-complement format with 1C resolution
  // s_fuel_gauge_settings.temp_thresh_max = CURRENT_SENSE_MAX_TEMP;

  s_fuel_gauge_settings.sense_resistor_mohms = SENSE_RESISTOR_MOHM;

  // Soft timer period for soc & chargin check
  s_current_storage->fuel_guage_cycle_ms = fuel_guage_cycle_ms;

  persist_init(&s_persist, CURRENT_SENSE_STORE_FLASH, &s_fuel_params, sizeof(s_fuel_params), false);
  status_ok_or_return(max17261_init(&s_fuel_guage_storage, &s_fuel_gauge_settings, &s_fuel_params));

  tasks_init_task(current_sense, TASK_PRIORITY(2), NULL);
  return STATUS_CODE_OK;
}
