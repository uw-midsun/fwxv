#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_mcu.h"
#include "log.h"
#include "steering_analog_task.h"
#include "steering_digital_task.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

#define DEVICE_ID 0x02

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
  wait_tasks(1);

  steering_analog_input();
  steering_digital_input();

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

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
    vTaskDelay(pdMS_TO_TICKS(1000));
    ++counter;
  }
}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  gpio_it_init();

  // Setup analog inputs and initialize adc
  steering_analog_adc_init();
  adc_init(ADC_MODE_SINGLE);
  steering_digital_input_init(master_task);

  can_init(&s_can_storage, &can_settings);
  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);
  tasks_start();

  return 0;
}
