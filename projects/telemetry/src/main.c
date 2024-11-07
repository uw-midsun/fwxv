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
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static UartSettings uart_settings = { .tx = { .port = GPIO_PORT_A, .pin = 2 },
                                      .rx = { .port = GPIO_PORT_A, .pin = 3 },
                                      .baudrate = 115200 };

TASK(CAN_TO_UART, TASK_STACK_256) {
  while (true) {
    uint8_t test = 0xFF;
    size_t test_len = sizeof(test);
    uart_tx(UART_PORT_2, &test, &test_len);
    CanMessage msg = { 0 };
    Datagram datagram = { .start_frame = 0xAA, .end_of_frame = 0xBB };

    // Polling can_storage queue
    StatusCode ret = can_queue_pop(&s_can_storage.rx_queue, &msg);

    if (ret == STATUS_CODE_OK) {
      decode_can_message(datagram, msg);

      // Typecast datagram object to pass into uart_tx
      uint8_t *uint8_datagram = (uint8_t *) &datagram; 
      size_t len = sizeof(uint8_datagram);
      StatusCode uart_ret = uart_tx(UART_PORT_2, uint8_datagram, &len);
      if (uart_ret != STATUS_CODE_OK) {
        LOG_DEBUG("UART_TX FAILED\n");
      }
    } else {
      // LOG_DEBUG("can_queue_pop failed\n");
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
  uart_init(UART_PORT_2, &uart_settings);

  init_master_task();
  can_init(&s_can_storage, &can_settings);

  tasks_init_task(CAN_TO_UART, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
