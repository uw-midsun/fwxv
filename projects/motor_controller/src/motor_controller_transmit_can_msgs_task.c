#include "motor_controller_transmit_can_msgs_task.h"

#include "log.h"
#include "mc_data.h"
#include "motor_controller_setters.h"
#include "tasks.h"

static SemaphoreHandle_t s_motor_controller_transmit_can_msgs_sem_handle;
static StaticSemaphore_t s_motor_controller_transmit_can_msgs_sem;

void run_motor_controller_transmit_can_msgs_cycle() {
  BaseType_t ret = xSemaphoreGive(s_motor_controller_transmit_can_msgs_sem_handle);

  if (ret == pdFALSE) {
    LOG_CRITICAL("mci transmit failed\n");
  }
}

TASK(transmit_can_msgs, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
    xSemaphoreTake(s_motor_controller_transmit_can_msgs_sem_handle, portMAX_DELAY);
    counter++;
    run_motor_controller_transmit_can_msgs_fast_cycle();
    if ((counter % 10) == 0) run_motor_controller_transmit_can_msgs_medium_cycle();
    if ((counter % 100) == 0) run_motor_controller_transmit_can_msgs_slow_cycle();
  }
}

void run_motor_controller_transmit_can_msgs_fast_cycle() {}

void run_motor_controller_transmit_can_msgs_medium_cycle() {
  set_motor_velocity_velocity_l(left_motor.velocity);
  set_motor_velocity_velocity_r(right_motor.velocity);

  set_motor_controller_vc_mc_voltage_l(left_motor.bus_voltage_v);
  set_motor_controller_vc_mc_current_l(left_motor.bus_current_a);
  set_motor_controller_vc_mc_voltage_r(right_motor.bus_voltage_v);
  set_motor_controller_vc_mc_current_r(right_motor.bus_current_a);

  set_mc_status_error_bitset_left(left_motor.mc_error_bitset);
  set_mc_status_error_bitset_right(right_motor.mc_error_bitset);
  set_mc_status_limit_bitset_left(left_motor.mc_limit_bitset);
  set_mc_status_limit_bitset_right(right_motor.mc_limit_bitset);
  // TODO(devAdhiraj): set_mc_status_board_fault_bitset();
  // TODO(devAdhiraj): set_mc_status_overtemp_bitset();

  set_motor_sink_temps_motor_temp_l(left_motor.motor_temp_c);
  set_motor_sink_temps_motor_temp_r(right_motor.motor_temp_c);
  set_motor_sink_temps_heatsink_temp_l(left_motor.heatsink_temp_c);
  set_motor_sink_temps_heatsink_temp_r(right_motor.heatsink_temp_c);

  set_dsp_board_temps_dsp_temp_l(left_motor.dsp_temp_c);
  set_dsp_board_temps_dsp_temp_r(right_motor.dsp_temp_c);
  send_task_end();
}

void run_motor_controller_transmit_can_msgs_slow_cycle() {}

StatusCode init_motor_controller_transmit_can_msgs() {
  status_ok_or_return(tasks_init_task(transmit_can_msgs, TASK_PRIORITY(2), NULL));
  return STATUS_CODE_OK;
}
