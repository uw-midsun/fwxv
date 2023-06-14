#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "tasks.h"
#include "uv_cutoff.h"
#include "uv_cutoff_getters.h"
#include "uv_cutoff_setters.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 100
#else
#define MASTER_MS_CYCLE_TIME 1000
#endif

// TODO(devAdhiraj): update with actual ports and pins
#define UV_CUTOFF_PORT GPIO_PORT_A
#define UV_CUTOFF_PIN 5

#define HORN_PORT GPIO_PORT_A
#define HORN_PIN 5

#define LIGHTS_PORT GPIO_PORT_A
#define LIGHTS_PIN 5

static const GpioAddress uv_status = { .port = UV_CUTOFF_PORT, .pin = UV_CUTOFF_PIN };

static const GpioAddress horn = { .port = HORN_PORT, .pin = HORN_PIN };

static const GpioAddress lights = { .port = LIGHTS_PORT, .pin = LIGHTS_PIN };

static CanStorage s_can_storage = { 0 };
static const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static UVCutoffState state = UV_CUTOFF_ACTIVE;

void uv_logic() {
  GpioState uv_value;

  if (gpio_get_state(&uv_status, &uv_value) != STATUS_CODE_OK) {
    LOG_CRITICAL("Error reading UV_cutoff pin!\n");
    return;
  }
  if (uv_value == GPIO_STATE_LOW) {
    state = UV_CUTOFF_DISCONNECTED;
  } else { 
    state = UV_CUTOFF_ACTIVE;
  }

  set_uv_cutoff_notification_signal1(state);

  if (state == UV_CUTOFF_DISCONNECTED) {
    return;
  }

  if (get_horn_and_lights_horn_state()) {
    gpio_set_state(&horn, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&horn, GPIO_STATE_LOW);
  }

  if (get_horn_and_lights_lights_state()) {
    gpio_set_state(&lights, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&lights, GPIO_STATE_LOW);
  }
}

void run_fast_cycle() {}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  uv_logic();

  run_can_tx_cycle(1);
  wait_tasks(1);
}

void run_slow_cycle() {}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
    ++counter;
    run_fast_cycle();
    if (!(counter % 10)) run_medium_cycle();
    if (!(counter % 100)) run_slow_cycle();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  tasks_init();
  log_init();
  can_init(&s_can_storage, &can_settings);
  gpio_init();

  gpio_init_pin(&uv_status, GPIO_INPUT_PULL_UP, GPIO_STATE_HIGH);
  gpio_init_pin(&horn, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);
  gpio_init_pin(&lights, GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_LOW);

  LOG_DEBUG("UV cutoff task!\n");

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
