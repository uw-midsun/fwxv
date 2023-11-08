#include "can.h"
#include "log.h"
#include "mppt.h"
#include "solar_sense_setters.h"
#include "tasks.h"

#define NUM_MPPTS 6

static CanStorage s_can_storage = { 0 };
static const CanSettings s_can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = CAN_CONTINUOUS,
};

// Turns on all the MPPTs
StatusCode init_mppts() {
  for (uint8_t i = 0; i < NUM_MPPTS; i++) {
    status_ok_or_return(mppt_init(i));
  }

  return STATUS_CODE_OK;
}

// Reads data from all MPPTs and sends it to telemetry
StatusCode read_mppts() {
  uint16_t current_data, voltage_data, pwm_data, status_data = 0;

  // Reads data from all MPPTs
  for (uint8_t i = 0; i < NUM_MPPTS; i++) {
    status_ok_or_return(mppt_read_current(i, &current_data));
    status_ok_or_return(mppt_read_voltage(i, &voltage_data));
    status_ok_or_return(mppt_read_voltage(i, &pwm_data));
    status_ok_or_return(mppt_read_status(i, &status_data));

    // Sets the appropriate CAN messages
    switch (i) {
      case 1:
        set_mppt_1_current(current_data);
        set_mppt_1_voltage(voltage_data);
        set_mppt_1_pwm(pwm_data);
        set_mppt_1_status(status_data);
        break;
      case 2:
        set_mppt_2_current(current_data);
        set_mppt_2_voltage(voltage_data);
        set_mppt_2_pwm(pwm_data);
        set_mppt_2_status(status_data);
        break;
      case 3:
        set_mppt_3_current(current_data);
        set_mppt_3_voltage(voltage_data);
        set_mppt_3_pwm(pwm_data);
        set_mppt_3_status(status_data);
        break;
      case 4:
        set_mppt_4_current(current_data);
        set_mppt_4_voltage(voltage_data);
        set_mppt_4_pwm(pwm_data);
        set_mppt_4_status(status_data);
        break;
      case 5:
        set_mppt_5_current(current_data);
        set_mppt_5_voltage(voltage_data);
        set_mppt_5_pwm(pwm_data);
        set_mppt_5_status(status_data);
        break;
      case 6:
        set_mppt_6_current(current_data);
        set_mppt_6_voltage(voltage_data);
        set_mppt_6_pwm(pwm_data);
        set_mppt_6_status(status_data);
        break;
    }
  }

  // Sends CAN messages
  status_ok_or_return(run_can_tx_cycle());
  status_ok_or_return(wait_tasks(1));

  return STATUS_CODE_OK;
}

TASK(mppt_task, TASK_STACK_512) {
  while (true) {
    read_mppts();
  }
}

int main() {
  log_init();

  LOG_DEBUG("Welcome to Solar Sense!");

  can_init(&s_can_storage, &s_can_settings);
  init_mppts();
  tasks_init();

  tasks_init_task(mppt_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
