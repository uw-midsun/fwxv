#include <stdio.h>

#include "adc.h"
#include "ads1015.h"
#include "can.h"
#include "can_board_ids.h"
#include "can_msg.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calib.c"
#include "pedal_setters.h"
#include "soft_timer.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_PEDAL,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

// TODO(Scrubpai): Read any GPIOs and the ADC for throttle
uint16_t read_pedal_throttle() {
  return false;
}

// TODO(Scrubpai): Read any GPIOs and the ADC for brake
bool read_pedal_brake() {
  return false;
}

void init_pedal_controls() {
  // Initialize GPIOs needed for the throttle
  interrupt_init();
  gpio_init();
  gpio_it_init();

  // Initializes ADC for ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GpioAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };
  adc_init(ADC_MODE_SINGLE);

  // Calibrates the pedal upon initialization
  pedal_calibrate();
}

void run_fast_cycle() {
  return;
}

void run_medium_cycle() {
  run_can_tx_cycle();
  wait_tasks(1);

  // Sending messages
  if (!read_pedal_brake()) {  // Brake is not pressed - Send proper throttle data
    set_pedal_output_brake_output(0);
    set_pedal_output_throttle_output(read_pedal_throttle());
  } else {  // Brake is pressed - Send proper brake data with throttle as 0
    set_pedal_output_brake_output(1);
    set_pedal_output_throttle_output(0);
  }
}

void run_slow_cycle() {
  return;
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
#ifdef TEST
    xSemaphoreTake(test_cycle_start_sem);
#endif
    run_fast_cycle();
    if (!(counter % 10)) run_medium_cycle();
    if (!(counter % 100)) run_slow_cycle();

#ifdef TEST
    xSemaphoreGive(test_cycle_end_sem);
#endif
    vTaskDelay(pdMS_TO_TICKS(100));
    ++counter;
  }
}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN!\n");
  init_pedal_controls();
  can_init(&s_can_storage, &can_settings);
  can_add_filter_in(SYSTEM_CAN_MESSAGE_PEDAL_PEDAL_OUTPUT);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);
  tasks_start();
  LOG_DEBUG("exiting main?\n");
  return 0;
}
