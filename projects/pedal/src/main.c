#include <stdio.h>

#include "adc.h"
#include "can.h"
#include "can_board_ids.h"
#include "can_msg.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "max11600.h"
#include "pedal_data.h"
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
  max11600_init(&s_max11600_storage, I2C_PORT_2);
}

void run_fast_cycle() {
  return;
}

void run_medium_cycle() {
  run_can_tx_cycle();
  wait_tasks(1);

  int16_t brake_position = INT16_MAX;
  int16_t throttle_position = 0;

  StatusCode status;

  status = read_brake_data(&brake_position);
  if (status == STATUS_CODE_OK) {
    read_throttle_data(&throttle_position);
  }

  // Only update data on STATUS_CODE_OK
  if (status == STATUS_CODE_OK) {
    // Sending messages
    if (!brake_position) {
      // Brake is not pressed - Send both readings, brake will be 0 and ignored by the receiver
      set_pedal_output_brake_output((uint32_t)brake_position);
      set_pedal_output_throttle_output((uint32_t)throttle_position);
    } else {
      // Brake is pressed - Send brake data with throttle as 0
      set_pedal_output_brake_output((uint32_t)brake_position);
      set_pedal_output_throttle_output(0);
    }
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
