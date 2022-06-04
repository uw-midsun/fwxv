#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_one_shot_mode_setters.h"
#include "log.h"
#include "tasks.h"

// Dsiable all threads
// pack message
// transmit message
// receive messahe
// unpack message
// all needed functions are in the slides

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

TASK(testing_task, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
    if (counter == 5) {
      //   set_transmit_msg1_status(0xab);
    }
    counter++;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  log_init();
  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);
  //   can_add_filter(SYSTEM_CAN_MESSAGE_NEW_CAN_TRANSMIT_MSG1);

  tasks_init_task(testing_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
