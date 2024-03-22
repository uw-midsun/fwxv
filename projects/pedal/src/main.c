#include <stdio.h>

#include "adc.h"
#include "calib.h"
#include "can.h"
#include "can_board_ids.h"
#include "can_msg.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "master_task.h"
#include "pedal_calib.h"
#include "pedal_data.h"
#include "pedal_setters.h"
#include "pedal_shared_resources_provider.h"
#include "soft_timer.h"
#include "tasks.h"

static const GpioAddress brake = BRAKE_LIMIT_SWITCH;
static const GpioAddress throttle = ADC_HALL_SENSOR;

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_PEDAL,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

// // These variables are passed to the shared resources provider, which then get used by the rest
// of the pedal project

static PedalCalibBlob s_calib_blob = { 0 };

void pedal_init() {
  // Initialize GPIOs needed for the throttle
  interrupt_init();
  calib_init(&s_calib_blob, sizeof(s_calib_blob), false);
  pedal_resources_init(calib_blob());
  pedal_data_init();
}

void pre_loop_init() {
  gpio_init_pin(&brake, GPIO_INPUT_PULL_DOWN, GPIO_STATE_LOW);
  gpio_init_pin(&throttle, GPIO_ANALOG, GPIO_STATE_LOW);
  adc_add_channel(throttle);
  adc_init();
}

void run_fast_cycle() {
  adc_run();
  GpioState brake_state = 0;
  int32_t throttle_position = 0;
  StatusCode status = gpio_get_state(&brake, &brake_state);

  if (status == STATUS_CODE_OK) {
    status = read_throttle_data(&throttle_position);
  }

  // Only update data on STATUS_CODE_OK
  if (status == STATUS_CODE_OK) {
    // Sending messages
    // if (brake_state == GPIO_STATE_LOW) {
    //   // Brake is pressed - Send brake data with throttle as 1
    //   set_pedal_output_brake_output(1);
    //   set_pedal_output_throttle_output(0);
    // } else {
    // Brake is not pressed
    uint32_t res = (uint32_t)throttle_position;
    set_pedal_output_brake_output(0);
    set_pedal_output_throttle_output(res);
  }
  // }

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_medium_cycle() {}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to PEDAL!\n");
  gpio_init();
  gpio_it_init();
  pedal_init();
  can_init(&s_can_storage, &can_settings);
  can_add_filter_in(SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT);

  init_master_task();
  tasks_start();
  LOG_DEBUG("exiting main?\n");
  return 0;
}
