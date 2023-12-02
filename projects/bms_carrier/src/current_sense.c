#include "current_sense.h"

#include <string.h>

#include "bms.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "log.h"
#include "soft_timer.h"
#include "gpio_it.h"
#include "interrupt.h"

#define MAX17261_I2C_PORT (I2C_PORT_1)
#define MAX17261_I2C_ADDR (0x6C)

// TODO (Adel C): Change these values to their actual values
#define CURRENT_SENSE_R_SENSE_U_OHMS (0.5)
#define MAIN_PACK_DESIGN_CAPACITY    (1.0f / CURRENT_SENSE_R_SENSE_U_OHMS)  // LSB = 5.0 (micro Volt Hours / R Sense)
#define MAIN_PACK_EMPTY_VOLTAGE      (512U)  // Only a 9-bit field, LSB = 78.125 (micro Volts)
#define CHARGE_TERMINATION_CURRENT   (1U)

static Max17261Storage s_fuel_guage_storage;
static CurrentStorage *s_current_storage;
static SoftTimer s_timer;
static bool s_is_charging;

// Periodically read and update the SoC of the car & update charging bool
static void prv_periodic_soc_read(SoftTimerId id) {
  StatusCode status = STATUS_CODE_OK;

  uint16_t soc = 0;
  uint16_t current = 0;
  status |= max17261_state_of_charge(&s_fuel_guage_storage, &soc);
  status |= max17261_curre(&s_fuel_guage_storage, &soc);

  if (status != STATUS_CODE_OK) {
    // Handle a fuel guage fault
  }

  // TODO (Adel): Send BMS SOC message

  // update s_is_charging
  // note that a negative value indicates the battery is charging
  s_is_charging = s_current_storage->average < 0;

  // Kick new soft timer
  soft_timer_init_and_start(s_current_storage->fuel_guage_cycle_ms, prv_periodic_soc_read, &s_timer);
}

bool current_sense_is_charging() {
  return s_is_charging;
}

StatusCode current_sense_init(CurrentStorage *storage, I2CSettings *settings,
                              uint32_t fuel_guage_cycle_ms, const Task *task) {
  interrupt_init();
  gpio_it_init();

  GpioAddress alrt_pin = {
    .port = GPIO_PORT_B,
    .pin = 1
  };

  InterruptSettings settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
    .edge = INTERRUPT_EDGE_RISING,
  };

  gpio_it_register_interrupt(&alrt_pin, &settings, ALRT_GPIO_IT, task);

  memset(storage, 0, sizeof(CurrentStorage));
  s_current_storage = storage;
  const Max17261Settings fuel_gauge_settings = {
    .charge_term_current = CHARGE_TERMINATION_CURRENT,
    .design_capacity = MAIN_PACK_DESIGN_CAPACITY,
    .empty_voltage = MAIN_PACK_EMPTY_VOLTAGE,
    .i2c_port = MAX17261_I2C_PORT, 
    .i2c_address = MAX17261_I2C_ADDR
  };

  // Soft timer period for soc & chargin check
  s_current_storage->fuel_guage_cycle_ms = fuel_guage_cycle_ms;

  status_ok_or_return(max17261_init(&s_fuel_guage_storage, &fuel_gauge_settings));
  soft_timer_init_and_start(s_current_storage->fuel_guage_cycle_ms, prv_periodic_soc_read, &s_timer);
  return STATUS_CODE_OK;
}
