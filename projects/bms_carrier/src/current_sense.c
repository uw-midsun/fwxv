#include "current_sense.h"

#include <string.h>

#include "bms.h"
#include "bms_carrier_setters.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "relays_fsm.h"
#include "soft_timer.h"
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
#define ALRT_PIN_V_RES_MICRO_V (400)

static Max17261Storage s_fuel_guage_storage;
static Max17261Settings s_fuel_gauge_settings;
static CurrentStorage *s_current_storage;
static SoftTimer s_timer;
static bool s_is_charging;
StatusCode fault_bitset = STATUS_CODE_OK;

StatusCode current_sense_fault_check() {
  return fault_bitset;
}

// Periodically read and update the SoC of the car & update charging bool
static StatusCode prv_fuel_gauge_read() {
  StatusCode status = STATUS_CODE_OK;

  uint16_t soc = 0;
  uint16_t current = 0;
  uint16_t voltage = 0;
  uint16_t temperature = 0;

  status |= max17261_state_of_charge(&s_fuel_guage_storage, &soc);
  status |= max17261_current(&s_fuel_guage_storage, &current);
  status |= max17261_voltage(&s_fuel_guage_storage, &voltage);
  status |= max17261_temp(&s_fuel_guage_storage, &temperature);

  if (status != STATUS_CODE_OK) {
    // TODO (Adel): Handle a fuel gauge fault
    // Open Relays
    fault_bitset |= status;
    return status;
  }

  if (temperature < 20) {
    pwm_set_dc(PWM_TIMER_3, (temperature * 100) / BMS_FAN_TEMP_THRESHOLD);
  } else {
    pwm_set_dc(PWM_TIMER_3, 100);
  }

  // Set Battery VT message signals
  set_battery_vt_batt_perc(soc);
  set_battery_vt_current(current);
  set_battery_vt_voltage(voltage);
  set_battery_vt_temperature(temperature);

  // update s_is_charging
  // note that a negative value indicates the battery is charging
  s_is_charging = s_current_storage->average < 0;

  return status;
}

TASK(current_sense, TASK_MIN_STACK_SIZE) {
  while (true) {
    uint32_t notification = 0;
    notify_wait(&notification, BLOCK_INDEFINITELY);
    LOG_DEBUG("Running Current Sense Cycle!\n");

    // Handle alert from fuel gauge
    if (notification & (1 << ALRT_GPIO_IT)) {
      // TODO (Adel): BMS Open Relays
      fault_bitset |= notification & (1 << ALRT_GPIO_IT);
    }

    prv_fuel_gauge_read();
    send_task_end();
  }
}

bool current_sense_is_charging() {
  return s_is_charging;
}

StatusCode run_current_sense_cycle() {
  StatusCode ret = notify(current_sense, CURRENT_SENSE_RUN_CYCLE);
  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode current_sense_init(CurrentStorage *storage, I2CSettings *i2c_settings,
                              uint32_t fuel_guage_cycle_ms) {
  interrupt_init();
  gpio_it_init();
  i2c_init(I2C_PORT_1, i2c_settings);

  GpioAddress alrt_pin = { .port = GPIO_PORT_A, .pin = 7 };

  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_RISING,
  };

  gpio_it_register_interrupt(&alrt_pin, &it_settings, ALRT_GPIO_IT, current_sense);

  memset(storage, 0, sizeof(CurrentStorage));
  s_current_storage = storage;

  s_fuel_gauge_settings.i2c_port = MAX17261_I2C_PORT;
  s_fuel_gauge_settings.i2c_address = MAX17261_I2C_ADDR;

  s_fuel_gauge_settings.charge_term_current = CHARGE_TERMINATION_CURRENT;
  s_fuel_gauge_settings.design_capacity = MAIN_PACK_DESIGN_CAPACITY;
  s_fuel_gauge_settings.empty_voltage = MAIN_PACK_EMPTY_VOLTAGE;

  // Expected MAX current / (uV / uOhmsSense) resolution
  s_fuel_gauge_settings.i_thresh_max =
      ((CURRENT_SENSE_MAX_CURRENT_A) / (ALRT_PIN_V_RES_MICRO_V / CURRENT_SENSE_R_SENSE_MILLI_OHMS));
  // Expected MIN current / (uV / uOhmsSense) resolution
  s_fuel_gauge_settings.i_thresh_min =
      ((CURRENT_SENSE_MIN_CURRENT_A) / (ALRT_PIN_V_RES_MICRO_V / CURRENT_SENSE_R_SENSE_MILLI_OHMS));
  // Interrupt threshold limits are stored in 2s-complement format with 1C resolution
  s_fuel_gauge_settings.temp_thresh_max = CURRENT_SENSE_MAX_TEMP;

  s_fuel_gauge_settings.r_sense_mohms = CURRENT_SENSE_R_SENSE_MILLI_OHMS;

  // Soft timer period for soc & chargin check
  s_current_storage->fuel_guage_cycle_ms = fuel_guage_cycle_ms;

  status_ok_or_return(max17261_init(&s_fuel_guage_storage, &s_fuel_gauge_settings));
  tasks_init_task(current_sense, TASK_PRIORITY(3), NULL);
  return STATUS_CODE_OK;
}
