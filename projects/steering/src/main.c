#include <stdbool.h>
#include <stdio.h>

#include "delay.h"
#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "log.h"
#include "master_task.h"
#include "steering_task.h"
#include "tasks.h"

#define DEVICE_ID 0x02

static const GpioAddress turn_signal_address = TURN_SIGNAL_GPIO;
static const GpioAddress cc_address = CC_CHANGE_GPIO;

void pre_loop_init() {
  steering_init();
  adc_init();
}


static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

void run_fast_cycle() {}

void run_medium_cycle() {
  run_can_rx_cycle();

  adc_run();
  steering_input();
  

  run_can_tx_cycle();

  
  LOG_DEBUG("Steering input lights: %d\n", g_tx_struct.steering_info_input_lights);
  LOG_DEBUG("Steering input cc: %d\n", g_tx_struct.steering_info_input_cc);

}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  gpio_it_init();
  
  // Setup analog inputs and initialize adc
  

  can_init(&s_can_storage, &can_settings);
  init_master_task();
  tasks_start();

  return 0;
}
