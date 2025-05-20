#include "telemetry.h"

#include "can.h"
#include "can_board_ids.h"
#include "datagram.h"
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "tasks.h"
#include "uart.h"

static TelemetryStorage *telemetry_storage;

static CanStorage s_can_storage = { 0 };
static const CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

TASK(can_message_processor, TASK_STACK_512) {
  Datagram tx_datagram = { 0U };
  CanMessage message = { 0U };
  StatusCode status = STATUS_CODE_OK;
  size_t datagram_length = 0U;
  uint32_t delay_time_ms = (1U / telemetry_storage->config->message_transmit_frequency_hz) * 1000U;

  while (true) {
    while (queue_receive(&s_can_storage.rx_queue.queue, &message, QUEUE_DELAY_BLOCKING) !=
           STATUS_CODE_OK) {
    }

    LOG_DEBUG("Received message\n");
    decode_can_message(&tx_datagram, &message);

    /* Wait for new data to be in the queue */
    datagram_length = tx_datagram.dlc + DATAGRAM_METADATA_SIZE;

    status = uart_tx(UART_PORT_2, (uint8_t *)&tx_datagram, &datagram_length);

    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Failed to transmit to telemetry transceiver!\n");
    }
  }
}

StatusCode telemetry_init(TelemetryStorage *storage, TelemetryConfig *config) {
  if (storage == NULL || config == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  telemetry_storage = storage;
  telemetry_storage->config = config;

  telemetry_storage->datagram_queue.item_size = sizeof(Datagram);
  telemetry_storage->datagram_queue.num_items = DATAGRAM_BUFFER_SIZE;
  telemetry_storage->datagram_queue.storage_buf = (uint8_t *)telemetry_storage->datagram_buffer;

  log_init();
  uart_init(telemetry_storage->config->uart_port, &telemetry_storage->config->uart_settings);
  can_init(&s_can_storage, &s_can_settings);
  queue_init(&telemetry_storage->datagram_queue);

  tasks_init_task(can_message_processor, TASK_PRIORITY(2), NULL);

  return STATUS_CODE_OK;
}
