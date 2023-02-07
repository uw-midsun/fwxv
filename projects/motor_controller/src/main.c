#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "delay.h"
#include "fsm.h"
#include "log.h"
#include "mci_fsm.h"
#include "mcp2515.h"
#include "misc.h"
#include "motor.h"
#include "soft_timer.h"
#include "tasks.h"

#ifdef MS_PLATFORM_X86
#define MASTER_MS_CYCLE_TIME 20
#else
#define MASTER_MS_CYCLE_TIME 200
#endif

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};
static Mcp2515Storage s_mcp2515_storage = { 0 };
Mcp2515Settings mcp2515_settings = {  // place holder values
  .spi_port = SPI_PORT_1,
  .spi_settings = {
    0
  },
  .interrupt_pin = { GPIO_PORT_A, 0 },
  .can_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .loopback = true,
  },
};

TASK(master_task, TASK_MIN_STACK_SIZE) {
  TickType_t previousWakeTime = xTaskGetTickCount();
  while (true) {
#ifdef TEST
    xSemaphoreTake(test_cycle_start_sem);
#endif
    run_can_rx_cycle();
    run_mcp2515_rx_cycle();
    wait_tasks(2);

    run_can_tx_cycle();
    run_mcp2515_tx_cycle();
    wait_tasks(2);
#ifdef TEST
    xSemaphoreGive(test_cycle_end_sem);
#endif
    vTaskDelayUntil(&previousWakeTime, pdMS_TO_TICKS(MASTER_MS_CYCLE_TIME));
  }
}

int main() {
  tasks_init();
  log_init();
  can_init(&s_can_storage, &can_settings);
  mcp2515_init(&s_mcp2515_storage, &mcp2515_settings);
  init_motor_controller_can();
  init_motor_controller_transmit_can_msgs();
  LOG_DEBUG("Motor Controller Task\n");

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
