#include "mcp2515.h"

#include <stddef.h>
#include <string.h>

#include "delay.h"
#include "gpio_it.h"
#include "log.h"

// Storage
static Mcp2515Storage *s_storage;

static SemaphoreHandle_t s_mcp2515_rx_sem_handle;
static StaticSemaphore_t s_mcp2515_rx_sem;

static SemaphoreHandle_t s_mcp2515_tx_sem_handle;
static StaticSemaphore_t s_mcp2515_tx_sem;

StatusCode run_mcp2515_rx_cycle() {
  BaseType_t ret = xSemaphoreGive(s_mcp2515_rx_sem_handle);

  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode run_mcp2515_tx_cycle() {
  BaseType_t ret = xSemaphoreGive(s_mcp2515_tx_sem_handle);

  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

TASK(MCP2515_RX, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
    xSemaphoreTake(s_mcp2515_rx_sem_handle, portMAX_DELAY);
    LOG_DEBUG("mcp2515_rx called: %d!\n", counter);
    counter++;

    mcp2515_rx_all();

    send_task_end();
  }
}

StatusCode mcp2515_receive(const CanMessage *msg) {
  StatusCode ret = can_queue_pop(&s_storage->rx_queue, msg);

  if (ret == STATUS_CODE_OK) {
    // LOG_DEBUG("Source Id: %d\n", msg->id);
    // LOG_DEBUG("Data: %lx\n", msg->data);
    // LOG_DEBUG("DLC: %ld\n", msg->dlc);
    // LOG_DEBUG("ret: %d\n", ret);
  }

  return ret;
}

TASK(MCP2515_TX, TASK_MIN_STACK_SIZE) {
  int counter = 0;
  while (true) {
    xSemaphoreTake(s_mcp2515_tx_sem_handle, portMAX_DELAY);
    LOG_DEBUG("mcp2515_tx called: %d!\n", counter);
    counter++;

    mcp2515_tx_all();

    send_task_end();
  }
}

StatusCode mcp2515_transmit(const CanMessage *msg) {
  if (s_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg->id.msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  }

  return mcp2515_hw_transmit(msg->id.raw, msg->extended, msg->data, msg->dlc);
}

static void no_op() {}

StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings) {
  memset(storage, 0, sizeof(*storage));
  s_storage = storage;

  if (mcp2515_rx_all == NULL) {
    mcp2515_rx_all = no_op;
  }
  if (mcp2515_tx_all == NULL) {
    mcp2515_tx_all = no_op;
  }

  mcp2515_hw_init(&storage->rx_queue, settings);

  // Create Semaphores
  s_mcp2515_rx_sem_handle = xSemaphoreCreateBinaryStatic(&s_mcp2515_rx_sem);
  configASSERT(s_mcp2515_rx_sem_handle);
  s_mcp2515_tx_sem_handle = xSemaphoreCreateBinaryStatic(&s_mcp2515_tx_sem);
  configASSERT(s_mcp2515_tx_sem_handle);

  status_ok_or_return(can_queue_init(&s_storage->rx_queue));

  if (settings->can_settings.mode == CAN_CONTINUOUS) {
    // Create RX and TX Tasks
    // ! Ensure the task priority is lower than the interrupt tasks in mcp2515_hw.c
    status_ok_or_return(tasks_init_task(MCP2515_RX, TASK_PRIORITY(2), NULL));
    status_ok_or_return(tasks_init_task(MCP2515_TX, TASK_PRIORITY(2), NULL));
  }

  return STATUS_CODE_OK;
}

// Just set the filters[i].raw, don't set individual parts of the id manually
// Otherwise it will be handled incorrectly when setting the filters
StatusCode mcp2515_set_filter(CanMessageId *filters, bool loopback) {
  return mcp2515_hw_set_filter(filters, loopback);
}
