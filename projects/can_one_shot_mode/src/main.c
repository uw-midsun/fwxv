#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_one_shot_mode_setters.h"
#include "log.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = 1,
};

TASK(testing_task, TASK_MIN_STACK_SIZE) {
  CanMessage msg = { 0 };
  while (true) {
    if (can_receive(&msg) == STATUS_CODE_OK) {
      LOG_DEBUG("can_one_shot_mode received a message\n");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  log_init();
  LOG_DEBUG("Welcome to CAN_ONE_SHOT_MODE!\n");
  can_init(&s_can_storage, &can_settings);

  can_add_filter_out(0x01a);

  tasks_init_task(testing_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
