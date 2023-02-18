#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "can_debug_getters.h"
#include "log.h"
#include "tasks.h"

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

int perform_calculation(uint8_t operation, uint8_t operandA, uint8_t operandB, uint8_t operandC) {
  switch (operation) {
    case 1:
      LOG_DEBUG("Hello World!\n");
      return 0;
      break;
    case 2:
      return operandA + operandB + operandC;
      break;
    case 3:
      return operandA - operandB - operandC;
      break;
    case 4:
      return operandA * operandB * operandC;
      break;
    case 5:
      return operandA / operandB / operandC;
      break;
    default:
      return 0;
      break;
  }
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  int result = 0;
  while (true) {
    run_can_rx_cycle();
    wait_tasks(1);

    result = perform_calculation(get_test_debug_operation(), get_test_debug_operandA(),
                                 get_test_debug_operandB(), get_test_debug_operandC());
    if (result) LOG_DEBUG("The result of the calculation is %d\n", result);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN!\n");
  can_init(&s_can_storage, &can_settings);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
