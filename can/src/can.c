#include "tasks.h"
#include "notify.h"
#include "semphr.h"
#include "event_groups.h"
// #include "can.h"
#include "can_codegen.h"
#include "can_watchdog.h"

#include "log.h"

rx_struct g_rx_struct;
tx_struct g_tx_struct;

static CanStorage *s_can_storage;

//takes 1 for filter_in, 2 for filter_out and default is unset
static int s_can_filter_in_en = 0;

TASK(CAN_RX, TASK_MIN_STACK_SIZE)
{
  int counter = 0;
  while (true)
  {
    notify_wait(NULL, BLOCK_INDEFINITELY);
    LOG_DEBUG("can_rx called: %d!\n", counter);
    counter++;

    can_rx_all();

    send_task_end();
  }
}

TASK(CAN_TX, TASK_MIN_STACK_SIZE)
{
  int counter = 0;
  while (true)
  {
    notify_wait(NULL, BLOCK_INDEFINITELY);
    LOG_DEBUG("can_tx called: %d!\n", counter);
    counter++;

    check_can_watchdogs();

    can_tx_all();

    clear_rx_received();

    send_task_end();
  }
}

StatusCode run_can_rx_cycle()
{
  StatusCode ret = notify(CAN_RX, 1);
  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode run_can_tx_cycle()
{
  StatusCode ret = notify(CAN_TX, 1);

  if (ret == pdFALSE) {
    return STATUS_CODE_INTERNAL_ERROR;
  }

  return STATUS_CODE_OK;
}

StatusCode can_init(CanStorage *storage, const CanSettings *settings)
{
  if (settings->device_id >= CAN_MSG_MAX_DEVICES)
  {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid device ID");
  }

  // Initializing storage
  memset(storage, 0, sizeof(*storage));
  storage->device_id = settings->device_id;

  s_can_storage = storage;

  // Initializing global structs
  memset(&g_rx_struct, 0, sizeof(g_rx_struct));
  memset(&g_tx_struct, 0, sizeof(g_tx_struct));

  status_ok_or_return(can_queue_init(&s_can_storage->rx_queue));
 
  // Initialize hardware settings
  status_ok_or_return(can_hw_init(&s_can_storage->rx_queue, settings));

  if (settings->mode == CAN_CONTINUOUS){
    // Create RX and TX Tasks 
    // TODO: Figure out priorities
    status_ok_or_return(tasks_init_task(CAN_RX, TASK_PRIORITY(2), NULL));
    status_ok_or_return(tasks_init_task(CAN_TX, TASK_PRIORITY(2), NULL));
  }
  return STATUS_CODE_OK;
}

StatusCode can_receive(const CanMessage *msg)
{
  // TODO: Figure out the ack_request
  StatusCode ret = can_queue_pop(&s_can_storage->rx_queue, msg);

  // if (ret == STATUS_CODE_OK)
  // {
  //   LOG_DEBUG("Source Id: %d\n", msg->id);
  //   LOG_DEBUG("Data: %lx\n", msg->data);
  //   LOG_DEBUG("DLC: %ld\n", msg->dlc);
  //   LOG_DEBUG("ret: %d\n", ret);
  // }

  return ret;
}

StatusCode can_transmit(const CanMessage *msg)
{
  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  return can_hw_transmit(msg->id.raw, msg->extended, msg->data_u8, msg->dlc);
}

StatusCode can_add_filter_in(CanMessageId msg_id) {
  //check if s_can_filter_in_en has been set
  if (s_can_filter_in_en == 0){
    s_can_filter_in_en = 1;
  }

  if (s_can_storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  } else if (msg_id >= CAN_MSG_MAX_IDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "CAN: Invalid message ID");
  } else if (s_can_filter_in_en != 1) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "CAN: CAN filter out is enabled already");
  }

  CanId can_id = { .raw = msg_id };
  CanId mask = { 0 };
  mask.raw = (uint32_t)~mask.msg_id;

  return can_hw_add_filter_in(mask.raw, can_id.raw, false);
}

StatusCode clear_rx_struct()
{
  memset(&g_rx_struct, 0, sizeof(g_rx_struct));
  return STATUS_CODE_OK;
}

StatusCode clear_tx_struct()
{
  memset(&g_tx_struct, 0, sizeof(g_tx_struct));
  return STATUS_CODE_OK;
}