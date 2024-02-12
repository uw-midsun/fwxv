#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CAN_COMMUNICATION,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
  // Initializing CAN in 1 shot mode to not loop
  .mode = CAN_ONE_SHOT_MODE,
};

static const GpioAddress s_leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

static void prv_init_leds() {
  for (size_t i = 0; i < SIZEOF_ARRAY(s_leds); i++) {
    gpio_init_pin(&s_leds[i], GPIO_OUTPUT_PUSH_PULL, GPIO_STATE_HIGH);
  }
}

static void prv_toggle_leds() {
  for (size_t i = 0; i < SIZEOF_ARRAY(s_leds); i++) {
    gpio_toggle_state(&s_leds[i]);
  }
}

TASK(testing_task, TASK_STACK_256) {
  CanMessage msg = { 0 };
  char log_message[16];
  uint8_t i = 0;
  uint8_t byte = 0;
  while (true) {
    if (can_receive(&msg) == STATUS_CODE_OK) {
      prv_toggle_leds();
      LOG_DEBUG("Received a message!\n");
      LOG_DEBUG("Data:\n");
      for (i = 0; i < msg.dlc; i++) {
        byte = msg.data >> (i * 8);
        snprintf(&log_message[i * 2], sizeof(log_message), "%x ", byte);
      }
      LOG_DEBUG("\t%s\n", log_message);

      // Echoing back out same msg
      can_transmit(&msg);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

#ifdef x86
int main(int argc, char *argv[]) {
  x86_main_init(atoi(argv[1]));
#else 
int main() {
#endif
  tasks_init();
  gpio_init();
  log_init();

  can_init(&s_can_storage, &can_settings);
  prv_init_leds();

  LOG_DEBUG("Welcome to CAN_ONE_SHOT_MODE!\n");

  // // Adding a filter
  // can_add_filter_in(0x5AD);

  tasks_init_task(testing_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
