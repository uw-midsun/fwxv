#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "datagram.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "uart.h"

static CanStorage s_can_storage = { 0 };

static const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_NEW_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static UartSettings uart_settings = { .tx = { .port = GPIO_PORT_B, .pin = 6 },
                                      .rx = { .port = GPIO_PORT_B, .pin = 7 },
                                      .baudrate = 115200 };

TASK(CAN_TO_UART, TASK_STACK_256) {
  while (true) {
    CanMessage msg = { 0 };
    Datagram datagram = { .start_frame = 0xAA, .end_of_frame = 0xBB };

    // Polling can_storage queue
    StatusCode ret = can_queue_pop(&s_can_storage.rx_queue, &msg);

    if (ret == STATUS_CODE_OK) {
      decode_can_message(datagram, msg);
      size_t len = sizeof(datagram);
      StatusCode uart_ret = uart_tx(UART_PORT_1, &datagram, &len);
    } else {
      LOG_DEBUG("can_queue_pop failed\n");
    }
  }
}

void run_fast_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  run_can_tx_cycle();
  wait_tasks(1);
}

Datagram decode_can_message(Datagram datagram, CanMessage msg) {
  // Populate datagram object with relevant data
  datagram.id = msg.id.raw;
  datagram.dlc = msg.dlc;
  datagram.data.data_u64 = msg.data;

  return datagram;
}

int main() {
  tasks_init();
  log_init();

  init_master_task();
  can_init(&s_can_storage, &can_settings);

  tasks_init_task(CAN_TO_UART, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
